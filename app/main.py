import os
from datetime import datetime
from uuid import uuid4

from uagents import Agent, Context, Protocol

from uagents_core.contrib.protocols.chat import (
    ChatAcknowledgement,
    ChatMessage,
    TextContent,
    chat_protocol_spec,
)

from grpc_client import generate_response

agent = Agent(
    name="FlashAI",
    port=8000,
    seed="Produhacks2026",
    endpoint=["https://truegradeofficial.com/submit"],
)

# Official Fetch chat protocol
chat_proto = Protocol(spec=chat_protocol_spec)


@agent.on_event("startup")
async def hi(ctx: Context):
    ctx.logger.info(f"Agent address: {agent.address}")


@chat_proto.on_message(ChatMessage)
async def handle_chat_message(ctx: Context, sender: str, msg: ChatMessage):
    user_text = None

    for item in msg.content:
        if isinstance(item, TextContent):
            user_text = item.text
            break

    if user_text is None:
        return

    # Send acknowledgement first
    ack = ChatAcknowledgement(
        timestamp=datetime.utcnow(),
        acknowledged_msg_id=msg.msg_id,
    )
    await ctx.send(sender, ack)

    # Process message with your gRPC backend
    try:
        result = generate_response(user_text)
        reply_text = f"gRPC Response: {result}"
    except Exception as e:
        reply_text = f"gRPC Error: {str(e)}"

    # Send chat response
    response = ChatMessage(
        timestamp=datetime.utcnow(),
        msg_id=uuid4(),
        content=[TextContent(type="text", text=reply_text)],
    )
    await ctx.send(sender, response)


@chat_proto.on_message(ChatAcknowledgement)
async def handle_ack(ctx: Context, sender: str, msg: ChatAcknowledgement):
    ctx.logger.info(
        f"Received acknowledgement from {sender} for message: {msg.acknowledged_msg_id}"
    )


agent.include(chat_proto, publish_manifest=True)

if __name__ == "__main__":
    agent.run()