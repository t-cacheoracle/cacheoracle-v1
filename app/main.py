import os
from typing import cast

import grpc
import cacheoracle_pb2
import cacheoracle_pb2_grpc

from fastapi import FastAPI
from uagents_core.contrib.protocols.chat import (
    ChatMessage,
    TextContent,
)
from uagents_core.envelope import Envelope
from uagents_core.identity import Identity
from uagents_core.utils.messages import parse_envelope, send_message_to_agent

name = "FlashAI"
from dotenv import load_dotenv

load_dotenv()
identity = Identity.from_seed(os.getenv("AGENT_SEED_PHRASE"), 0)

app = FastAPI()

@app.get("/status")
async def healthcheck():
    return {"status": "OK - Agent is running"}

@app.post("/chat")
async def handle_message(env: Envelope):
    msg = cast(ChatMessage, parse_envelope(env, ChatMessage))
    user_text = msg.text()

    print(f"Received message from {env.sender}: {user_text}")

    # call (gRPC)
    backend_reply = call_backend(user_text)

    send_message_to_agent(
        destination=env.sender,
        msg=ChatMessage([TextContent(backend_reply)]),
        sender=identity,
    )

def call_backend(user_input: str) -> str:
    with grpc.insecure_channel("localhost:50051") as channel:
        stub = cacheoracle_pb2_grpc.CodegenServiceStub(channel)

        request = cacheoracle_pb2.GenerateRequest(prompt=user_input)
        response = stub.GeneratePython(request)

        return response.python_code