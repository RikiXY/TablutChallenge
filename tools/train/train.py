# uv run train.py POSITIONS OUT.nnue [--lam 0.5] [--epochs 8] [--lr 1e-3] [--init OLD.nnue]

import argparse
import copy
import math

import numpy as np
import torch

INPUTS = 244  # king 0-80, white 81-161, black 162-242, White to move 243
HIDDEN = 128
PAD = INPUTS
MAX_ACTIVE = 26  # king + 8 white + 16 black + side
K = math.log(10) / 400

parser = argparse.ArgumentParser()
parser.add_argument("positions")
parser.add_argument("out")
parser.add_argument("--lam", type=float, default=0.0, help="weight of the search score in the target")
parser.add_argument("--epochs", type=int, default=8)
parser.add_argument("--lr", type=float, default=1e-3)
parser.add_argument("--batch", type=int, default=4096)
parser.add_argument("--init", help="start from this network instead of random weights")
args = parser.parse_args()
device = "cuda" if torch.cuda.is_available() else "cpu"


def symmetric(square, s):
    """s: bit 0 flips the rows, bit 1 the columns, bit 2 swaps them"""
    row, col = divmod(square, 9)
    if s & 1:
        row = 8 - row
    if s & 2:
        col = 8 - col
    if s & 4:
        row, col = col, row
    return row * 9 + col


# SYMMETRY[s][input] is the input after symmetry s (the side and PAD don't move)
SYMMETRY = torch.tensor(
    [[i // 81 * 81 + symmetric(i % 81, s) if i < 243 else i for i in range(INPUTS + 1)] for s in range(8)], device=device
)

# Line: IF_TEST WHITE_SCORE EVALUATION_SCORE SIDE KING WHITE_SQUARES BLACK_SQUARES [SEARCH_SCORE]
data = {0: [], 1: []}
for line in open(args.positions):
    words = line.split()
    inputs = [int(words[4])] + ([243] if words[3] == "0" else [])
    for squares, offset in ((words[5], 81), (words[6], 162)):
        if squares != "-":
            inputs += [offset + int(square) for square in squares.split(",")]
    result = float(words[1])
    target = result
    if args.lam:
        target = args.lam / (1 + math.exp(-K * float(words[7]))) + (1 - args.lam) * result
    data[int(words[0])].append((inputs + [PAD] * (MAX_ACTIVE - len(inputs)), float(words[2]), result, target))


def tensors(rows):
    return [torch.tensor([row[i] for row in rows], device=device) for i in range(4)]


train_inputs, train_linear, train_result, train_target = tensors(data[0])
test_inputs, test_linear, test_result, _ = tensors(data[1])
print(f"train {len(train_result)}, test {len(test_result)}, on {device}")


class Network(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.input = torch.nn.EmbeddingBag(INPUTS + 1, HIDDEN, mode="sum", padding_idx=PAD)
        self.hidden_bias = torch.nn.Parameter(torch.full((HIDDEN,), 0.5))
        self.output = torch.nn.Linear(HIDDEN, 1)
        torch.nn.init.normal_(self.input.weight, std=0.1)
        torch.nn.init.zeros_(self.output.weight)
        torch.nn.init.zeros_(self.output.bias)
        with torch.no_grad():
            self.input.weight[PAD].zero_()

    def forward(self, inputs, linear):
        hidden = torch.clamp(self.input(inputs) + self.hidden_bias, 0, 1)
        return linear + self.output(hidden).squeeze(1) * 400

    def parameters_in_file_order(self):
        return self.input.weight[:INPUTS], self.hidden_bias, self.output.weight[0], self.output.bias


def loss(score, target):
    return torch.nn.functional.binary_cross_entropy_with_logits(K * score, target)


network = Network().to(device)
if args.init:
    weights = torch.tensor(np.fromfile(args.init, dtype="<f4"), device=device)
    assert weights.numel() == (INPUTS + 2) * HIDDEN + 1, "wrong network size"
    with torch.no_grad():
        for parameter in network.parameters_in_file_order():
            parameter.copy_(weights[: parameter.numel()].view_as(parameter))
            weights = weights[parameter.numel():]

def test_loss():
    with torch.no_grad():
        return loss(network(test_inputs, test_linear), test_result).item()


optimizer = torch.optim.Adam(network.parameters(), lr=args.lr)
best_loss = test_loss()
best_weights = copy.deepcopy(network.state_dict())
best_epoch = 0
print(f"test loss: linear {loss(test_linear, test_result).item():.5f}, start {best_loss:.5f}")
for epoch in range(args.epochs):
    order = torch.randperm(len(train_result), device=device)
    for start in range(0, len(order), args.batch):
        batch = order[start : start + args.batch]
        symmetries = SYMMETRY[torch.randint(0, 8, (len(batch),), device=device)]
        inputs = torch.gather(symmetries, 1, train_inputs[batch])
        optimizer.zero_grad()
        loss(network(inputs, train_linear[batch]), train_target[batch]).backward()
        optimizer.step()
    epoch_loss = test_loss()
    print(f"epoch {epoch + 1}: test loss {epoch_loss:.5f}", flush=True)
    if epoch_loss < best_loss:
        best_loss = epoch_loss
        best_weights = copy.deepcopy(network.state_dict())
        best_epoch = epoch + 1

print(f"best test loss {best_loss:.5f} at epoch {best_epoch}")
network.load_state_dict(best_weights)

with open(args.out, "wb") as file, torch.no_grad():
    for parameter in network.parameters_in_file_order():
        file.write(parameter.cpu().numpy().astype("<f4").tobytes())