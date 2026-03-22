from uagents import Agent, Context, Protocol, Model
import random
from uagents import Field
import sys
from grpc_client import generate_response

agent = Agent(
    name="FalshAI",
    port=8000,
    seed="Produhacks2026",
    endpoint=["http://YOUR_IP:8000/submit"],
)


@agent.on_event("startup")
async def hi(ctx: Context):
    ctx.logger.info(agent.address)


class Request(Model):
    prompt: str = Field(description="Prompt for the gRPC service")

class Response(Model):
    text: str = Field(description="Response from gRPC service")



grpc_protocol = Protocol("GRPCService")


@grpc_protocol.on_message(model=Request, replies={Response})
async def handle_grpc_call(ctx: Context, sender: str, msg: Request):
    try:
        result = generate_response(msg.prompt)
        message = f"gRPC Response: {result}"
    except Exception as e:
        message = f"gRPC Error: {str(e)}"
    
    await ctx.send(
        sender, Response(text=message)
    )


agent.include(grpc_protocol, publish_manifest=True)

agent.run()
