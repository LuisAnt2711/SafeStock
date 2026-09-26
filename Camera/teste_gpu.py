import torch
import easyocr

print("PyTorch:", torch.__version__)
print("CUDA:", torch.cuda.is_available())

if torch.cuda.is_available():
    print("GPU:", torch.cuda.get_device_name(0))

reader = easyocr.Reader(['en'], gpu=True)

print("EasyOCR inicializado com GPU.")