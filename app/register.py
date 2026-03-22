import os
from uagents_core.crypto import Identity
from fetchai.registration import register_with_agentverse

# Store your Agentverse API Key in the environment variables. 
AGENTVERSE_KEY = "eyJhbGciOiJSUzI1NiJ9.eyJleHAiOjE3NzQxNjk2NzMsImlhdCI6MTc3NDE2NjA3MywiaXNzIjoiZmV0Y2guYWkiLCJqdGkiOiJkYjk0ZWFiNDNiYTE4ZmE1Mjk4OTE1ZTgiLCJzY29wZSI6IiIsInN1YiI6IjAyZTgzYjNkMmJhNWZlNzEyYjliZWI1NTExZGRlYzZiZTk5N2E4MTE1MmNlNzg5NCJ9.nbrjm0O_mJCGdlndLBSWTc6r8rgnNOLGcKBSMghRfZVDboe7isOukXOaLtCk3Hvl9YStryoxFhcfgZtg5z9tcwzeMXjqwvHdt16Bl9NCQHklqu-1v1n6hiPi7aO6v3qLB2--IpX_EfuLKzpf8IXg8kvUBBcEJBGuW40OMRbWichndwqsQjcm3O8xIvRL7hqzzbMKCL99FF-ofh3ply11aNeu5bBAtumBr4yvszhI-6hLdDC2Xzi5Y50u-KcMwtLXaZmwJa_JJ8a9d5fFsinXnKgEC9bE-5JcN7iQghpBGU6o_hmHxoJR7DIC74VfxKj2jaWd9cBafEnA3mIW1EPJaQ"

# Your Agent's unique key for generating an address on Agentverse
ai_identity = Identity.from_seed(("Produhacks2026"), 0)

# Give your Agent a name. This allows you to easily identify one
# of your Agents from other agents on Agentverse.
name = "FlashAI"

# The webhook that your AI receives messages on.
ai_webhook = "https://truegradeofficial.com/submit"

register_with_agentverse(
    ai_identity,
    ai_webhook,
    AGENTVERSE_KEY,
    name,
    readme="HI",
)