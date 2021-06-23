import numpy as np
import torch
from speechbrain.pretrained import SpeakerRecognition
from speechbrain.pretrained import EncoderClassifier

# Threshold calculated by taking average of highest between sets comparison and lowest within
# sets comparison of testing data
threshold = 0.35195

def encodeAudioData(rawData):
    classifier = EncoderClassifier.from_hparams(source="speechbrain/spkrec-ecapa-voxceleb")
    # Trim zeros and sign extend to int32_t
    rawData = np.trim_zeros(rawData)
    rawData = np.right_shift(rawData, 6)
    rawData = np.array([sign_extend(x, 18) for x in rawData]).astype(np.int32)

    # Turn into pytorch tensor
    audioTensor = torch.from_numpy(rawData)

    # Calculate embedding
    emb = classifier.encode_batch(audioTensor, normalize=True)

    return emb

def validateAudioData(rawData, user_tensor):
    verification = SpeakerRecognition.from_hparams(source="speechbrain/spkrec-ecapa-voxceleb")

    test_tensor = encodeAudioData(rawData)
    score = verification.similarity(user_tensor, test_tensor)
    match = True if (score > threshold) else False

    return score, match

def processSimpleAudio(rawData):
    # Trim zeros and sign extend to int32_t
    rawData = np.trim_zeros(rawData)
    rawData = np.right_shift(rawData, 6)
    rawData = np.array([sign_extend(x, 18) for x in rawData]).astype(np.int32)

    # Convert audio to float
    rawData = rawData.astype(np.float64)

    # Normalize audio to int16_t range max
    rawData *= 32767 / max(abs(rawData))

    # simpleaudio requires input to be int16_t
    return rawData.astype(np.int16)

def sign_extend(value, bits):
    sign_bit = 1 << (bits - 1)
    return (value & (sign_bit - 1)) - (value & sign_bit)
