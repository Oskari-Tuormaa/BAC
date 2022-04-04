"""A minimal, self-contained example of using pydwf."""

from pydwf import DwfLibrary, DwfAnalogOutNode, DwfAnalogOutFunction
import pydwf
from pydwf.utilities import openDwfDevice
import numpy as np
import matplotlib.pyplot as plt
import time
from scipy.signal import butter, lfilter

outCH = [0, 1]  # Channel numbering starts at zero.
inCH = [0, 1]
node = DwfAnalogOutNode.Carrier

dwf = DwfLibrary()


def generate_apesw(n_warmup: int, n_pulse: int):
    warmup = [*[1, 0] * n_warmup]
    switch = [1, 1, 0, 1, 0]
    return [*((warmup + switch) * n_pulse) + warmup]


def butter_bandpass(lowcut, highcut, fs, order=5):
    return butter(order, [lowcut, highcut], fs=fs, btype='band')


def butter_bandpass_filter(data, lowcut, highcut, fs, order=5):
    b, a = butter_bandpass(lowcut, highcut, fs, order=order)
    y = lfilter(b, a, data)
    return y


apesw = generate_apesw(6, 3)
n = len(apesw)
freq = 80e3 / n

with openDwfDevice(dwf) as device:
    # Setup analogIn
    device.analogIn.reset()
    for CH in inCH:
        device.analogIn.channelEnableSet(CH, True)
    device.analogIn.acquisitionModeSet(pydwf.DwfAcquisitionMode.Record)
    device.analogIn.frequencySet(1e6)
    device.analogIn.recordLengthSet(5 / freq)
    time.sleep(2)

    # Setup analogOut
    for CH in outCH:
        device.analogOut.reset(CH)
        device.analogOut.nodeEnableSet(CH, node, True)
        device.analogOut.nodeFunctionSet(CH, node, DwfAnalogOutFunction.Custom)
        device.analogOut.nodeFrequencySet(CH, node, freq)
        device.analogOut.nodeAmplitudeSet(CH, node, 5)
        device.analogOut.idleSet(CH, pydwf.DwfAnalogOutIdle.Disable)
        device.analogOut.runSet(CH, 1 / freq)
        device.analogOut.waitSet(CH, 0 / freq)
        device.analogOut.repeatSet(CH, 1)
        device.analogOut.triggerSourceSet(CH, pydwf.DwfTriggerSource.AnalogIn)
    device.analogOut.nodeDataSet(outCH[0], node, np.array(apesw))
    device.analogOut.nodeDataSet(outCH[1], node, 1 - np.array(apesw))

    fig, axs = plt.subplots(2, 1, sharex=True)
    # for ax in axs:
    #     ax.set_xlim([0, 1000])
    #     ax.set_ylim([-5, 5])
    lines = [ax.plot([])[0] for ax in axs]
    plt.show(block=False)

    # Run multiple accquisitions
    while True:
        print("Updating")

        for CH in outCH:
            device.analogOut.configure(CH, True)
        device.analogIn.configure(False, True)
        samples = []
        while True:
            status = device.analogIn.status(True)
            samples_avail, _, _ = device.analogIn.statusRecord()

            if samples_avail != 0:
                curr_samples = np.vstack(
                    [device.analogIn.statusData(idx, samples_avail)
                     for idx in inCH]).transpose()
                samples.append(curr_samples)

            if status == pydwf.DwfState.Done:
                break

        samples = np.concatenate(samples)

        for i, line in enumerate(lines):
            data = samples[:, i]
            if i == 1:
                data = butter_bandpass_filter(data, 20e3, 80e3, 1e6)
            line.set_data(range(len(data)), data)
            axs[i].set_ylim([min(data), max(data)])
            axs[i].set_xlim([0, len(data)])

        fig.canvas.draw()
        plt.pause(1)
