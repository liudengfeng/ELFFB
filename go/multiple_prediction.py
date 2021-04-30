import torch
import torch.nn as nn
from torch.autograd import Variable
import math

from rlpytorch import ArgsProvider, add_err
from rlpytorch.trainer import topk_accuracy

class MultiplePrediction:
    def __init__(self):
        self.args = ArgsProvider(
            call_from = self,
            define_args = [
                ("multipred_no_backprop", dict(action="store_true")),
            ],
        )

        self.policy_loss = nn.NLLLoss().cuda()
        self.value_loss = nn.MSELoss().cuda()

    def update(self, mi, batch, stats):
        ''' Update given batch '''
        # Current timestep.
        state_curr = mi["model"](batch.hist(0))
        total_loss = None
        eps = 1e-6
        targets = batch.hist(0)["offline_a"]
        for i, pred in enumerate(state_curr["pis"]):
            if i == 0:
                prec1, prec5 = topk_accuracy(pred.data, targets[:, i].contiguous(), topk=(1, 5))
                stats["top1_acc"].feed(prec1[0])
                stats["top5_acc"].feed(prec5[0])

            # backward.
            loss = self.policy_loss((pred + eps).log(), Variable(targets[:, i]))
            stats["loss" + str(i)].feed(loss.data[0])
            total_loss = add_err(total_loss, loss / (i + 1))

        stats["total_loss"].feed(total_loss.data[0])
        if not self.args.multipred_no_backprop:
            total_loss.backward()
