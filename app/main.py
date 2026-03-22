import os
from typing import cast
from fastapi import FastAPI
from uagents_core.contrib.protocols.chat import (
    ChatMessage,
    TextContent,
)
from uagents_core.envelope import Envelope
from uagents_core.identity import Identity
from uagents_core.utils.messages import parse_envelope, send_message_to_agent

name = "FlashAI"
identity = Identity.from_seed(os.environ["AGENT_SEED_PHRASE"], 0)
readme = "Make agent faster"
endpoint = "https://cast-stamps-grad-stress.trycloudflare.com/chat"

app = FastAPI()

@app.get("/status")
async def healthcheck():
    return {"status": "OK - Agent is running"}

@app.post("/chat")
async def handle_message(env: Envelope):
    try:
        msg = cast(ChatMessage, parse_envelope(env, ChatMessage))
        user_input = msg.text()

        print(f"Received: {user_input}")

        # ---- simulate gRPC call ----
        grpc_response = fake_grpc_call(user_input)

        # ---- send response back ----
        send_message_to_agent(
            destination=env.sender,
            msg=ChatMessage([TextContent(grpc_response)]),
            sender=identity,
        )

        return {"status": "success"}

    except Exception as e:
        return {"status": "error", "message": str(e)}


def fake_grpc_call(input_text: str) -> str:
    # simulate backend model
    return f"Processed: {input_text}"