import asyncio
import websockets
import io
from PIL import Image
import numpy as np
import cv2

WS_URL = "ws://127.0.0.1:8081"
async def handle_ws():
    async with websockets.connect(WS_URL) as ws:
        print(f"Connected to {WS_URL}")
        while True:
            try:
                message = await ws.recv()
                if isinstance(message, str):
                    print(message)
                elif isinstance(message, bytes):
                    try:
                        image = Image.open(io.BytesIO(message))
                        cv_image = cv2.cvtColor(np.array(image), cv2.COLOR_RGB2BGR)
                        cv2.imshow("WebSocket Image", cv_image)
                        cv2.waitKey(1)
                    except Exception as e:
                        print(e)
            except websockets.ConnectionClosed:
                break

asyncio.run(handle_ws())