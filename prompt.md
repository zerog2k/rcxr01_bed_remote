# prompt 1 - refactoring
Your role: You are an embedded software engineer. Ask before making any crucial decisions. Document as much as possible along the way. Group related changes together as much as possible, such that they can be made into atomic commits.

The desired system operation is in `OPERATION.md`.
You can find platfromio virtualenv in  /Users/jens/.platformio/penv

Your tasks:
- Examine the repo and understand what is going on.
- Refactor the code-base in order to simplify the `main.cpp`. 
- test compilation with platformio, and resolve issues that you fine.


# prompt 2 - use menuing system
Your role: You are an embedded software engineer. Ask before making any crucial decisions. Document as much as possible along the way. Group related changes together as much as possible, such that they can be made into atomic commits.

The desired system operation is in `OPERATION.md`.
You can find platfromio virtualenv in  /Users/jens/.platformio/penv

Your tasks:
- understand how the code interacts with u8x8 library to drive the lcd display.
- refactor the u8x8 code to use u8g2 instead
