import pint
import scipy.io.wavfile as sci_wav
import numpy as np

ureg = pint.UnitRegistry()

pwf = [*[1, 0]*6]
iwf = [1, 1, 0, 1, 0]

awf = (pwf + iwf) * 3 + pwf

print("".join([str(x) for x in np.array(awf)] + ["0"]     * 50))
print("".join([str(x) for x in np.array(awf)] + ["1"]     * 50))
print("".join([str(x) for x in (1-np.array(awf))] + ["0"] * 50))
print("".join([str(x) for x in (1-np.array(awf))] + ["1"] * 50))

apesw_npn_wf = np.array(awf + [0] * 400)
apesw_pnp_wf = 1 - np.array(awf + [1] * 400)

sci_wav.write("APESW_npn.wav", int(80e3), np.array(apesw_npn_wf).astype(np.int16) * (2**15 - 1))
sci_wav.write("APESW_pnp.wav", int(80e3), np.array(apesw_pnp_wf).astype(np.int16) * (2**15 - 1))

f = 40 * ureg.kHz

print(len(apesw_pnp_wf))
print((2 * f / len(apesw_pnp_wf)).to(ureg.Hz))

vs = 343 * ureg.m / ureg.s
x = 625 * ureg.us
print((vs * x).to(ureg.cm))
