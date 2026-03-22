from datetime import datetime, timezone
from uuid import uuid4

import grpc
import cacheoracle_pb2
import cacheoracle_pb2_grpc

from uagents import Agent, Context, Protocol
from uagents_core.contrib.protocols.chat import (
    ChatAcknowledgement,
    ChatMessage,
    EndSessionContent,
    TextContent,
    chat_protocol_spec,
)

agent = Agent(
    name="FlashLLM",
    seed="Produhacks2026-FlashLLM",
    port=8005,
    mailbox=True,
    publish_agent_details=True,
)

chat_proto = Protocol(spec=chat_protocol_spec)


def generate_response(prompt: str) -> str:
    channel = grpc.insecure_channel("localhost:50051")
    stub = cacheoracle_pb2_grpc.CodegenServiceStub(channel)

    request = cacheoracle_pb2.GenerateRequest(prompt=prompt)
    response = stub.GenerateResponse(request)

    return response.response


@agent.on_event("startup")
async def startup(ctx: Context):
    ctx.logger.info("FlashLLM started")
    ctx.logger.info(f"Agent address: {agent.address}")


@chat_proto.on_message(ChatMessage)
async def handle_chat_message(ctx: Context, sender: str, msg: ChatMessage):
    print("HIT CHAT MESSAGE", flush=True)
    ctx.logger.info(f"Received ChatMessage from {sender}")

    await ctx.send(
        sender,
        ChatAcknowledgement(
            timestamp=datetime.now(timezone.utc),
            acknowledged_msg_id=msg.msg_id,
        ),
    )

    user_text = " ".join(
        item.text for item in msg.content if isinstance(item, TextContent)
    ).strip()

    print(f"USER TEXT: {user_text}", flush=True)
    ctx.logger.info(f"User text: {user_text}")

    try:
        reply_text = generate_response(user_text)
    except Exception as e:
        ctx.logger.error(f"gRPC error: {e}")
        reply_text = f"gRPC error: {str(e)}"

    await ctx.send(
        sender,
        ChatMessage(
            timestamp=datetime.now(timezone.utc),
            msg_id=uuid4(),
            content=[
                TextContent(type="text", text=reply_text),
                EndSessionContent(type="end-session"),
            ],
        ),
    )


@chat_proto.on_message(ChatAcknowledgement)
async def handle_ack(ctx: Context, sender: str, msg: ChatAcknowledgement):
    ctx.logger.info(
        f"Received acknowledgement from {sender} for {msg.acknowledged_msg_id}"
    )


agent.include(chat_proto, publish_manifest=True)

if __name__ == "__main__":
    agent.run()