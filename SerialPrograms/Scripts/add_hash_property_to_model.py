import onnx
import hashlib
import sys

# You can use any other hash algorithms to ensure the model and its hash-value is a one-one mapping. 
def hash_file(file_path:str, algorithm:str='sha256', chunk_size:int=8192):
    hash_func = hashlib.new(algorithm)
    with open(file_path, 'rb') as file:
        while chunk := file.read(chunk_size):
            hash_func.update(chunk)
    return hash_func.hexdigest()

CACHE_KEY_NAME = "CACHE_KEY"


if len(sys.argv) == 1:
    print(f"Usage: {sys.argv[0]} <model_path>")
    exit(0)

model_path = sys.argv[1]
print(f"Adding a cache value to the metadata_props of the model: {model_path}")

if not model_path.endswith(".onnx"):
    print(f"Error: model path is not an onnx file")

m = onnx.load(model_path)

cache_key = m.metadata_props.add()
cache_key.key = CACHE_KEY_NAME
cache_key.value = str(hash_file(model_path))

print(f"Added key {CACHE_KEY_NAME} and value {cache_key.value}")

onnx.save_model(m, model_path)

print(f"Saved model to the same path, {model_path}")