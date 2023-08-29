# Solana Unreal SDK


![bifrost-solana](https://github.com/Bifrost-Technologies/Solana-Unreal-SDK/assets/24855008/fbd7075b-2d04-487d-aba3-5147e8aa327e)



A complete Unreal Engine plugin for building blockchain games on Solana with Unreal Engine 5.

The toolkit comes with a starting template for MMO Solana integrated games, a multiplayer game server & client SDK, and REST API to handle global server calls for players.

-Full SOLNET capabilities in C++ & blueprint

-In-game wallet system with data protection and encryption

-Mint metaplex NFTs

-Payment Processor Examples

-Universal platform support when using the REST API

-Unreal SOLNET for on-chain game development

Using Unreal SOLNET, we can use [SOL.NET](https://github.com/bmresearch/Solnet) and the power of .NET runtime inside the game giving developers the ability to communicate with the Solana blockchain without having to build an entire c++ library from scratch.

 For specific application types like VR/AR/XR where an in-game wallet would be needed then we use UnrealSOLNET to call the Solana UE5 SDK directly in the game.
 Using [LinkStream](https://github.com/Bifrost-Technologies/Link-Stream) the engine is capable of communicating with .NET runtime, while also being able to request signing via [Tranquility Wallet](https://github.com/Bifrost-Technologies/Tranquility) and popular mobile wallets using encrypted TCP & Websocket loopback mechanics.

## Documentation

![HighresScreenshot00008](https://github.com/Bifrost-Technologies/Solana-Unreal-SDK/assets/24855008/cbf305f6-3369-4a23-b801-67ae1168d88a)

### General Usage

All plugin related content can be found in the content browser under: Plugins -> Solana SDK

First start by adding a Linkstream blueprint to your games level. 
It can be found in: Plugins -> Solana SDK Content -> Blueprints

Once you have linkstream in your level you can communicate with Unreal Solnet with fully capabilities.

Open up the level blueprint and create a sequence that initializes the chain client first
![image](https://github.com/Bifrost-Technologies/Solana-Unreal-SDK/assets/24855008/a67023e0-3622-461c-b0ff-b534e717abcf)


Create custom events for connecting to Unreal SOLNET SDK. Several requests are implemented by default for general usage of the SDK, but it is recommended you make your own wrapper that fits your needs.
![image](https://github.com/Bifrost-Technologies/Solana-Unreal-SDK/assets/24855008/d808c045-77ea-4f4a-8b3b-b36753419be2)

Bind events from objects in the world to invoke the chain client from anywhere
![image](https://github.com/Bifrost-Technologies/Solana-Unreal-SDK/assets/24855008/0e89c09a-7af4-47f8-8089-d3b466b862e8)

For a more in-depth look on how to properly setup the Solana SDK check out the sandbox map in the Maps folder found with the rest of the content and review the Solana SDK source code for better understanding on how the SDK interacts with the framework.

### Custom Linkstream Wrapper - Advanced Usage
To expand the wrapper and make full usage of Unreal SOLNET you will need to move the SolanaKit source to a location outside of the plugin folder. 

All marketplace plugins can be found in the 5.2 Engine folder in Plugins -> Marketplace -> Solana SDK -> SolanaKit

After moving the source to a new workspace, open up the project and navigate to GameRuntime.cs then scroll to the bottom method called HandleRequestEvent

C#
![image](https://github.com/Bifrost-Technologies/Solana-Unreal-SDK/assets/24855008/5dd39224-a174-4993-a611-3d76b4b53a20)

This is the LinkStream wrapper its where you can expand and add more requests for what ever you need. This is an example from the Frontiers SDK that shows how requests were added to handle on-chain program transactions for a PvP RTS game

C#
![image](https://github.com/Bifrost-Technologies/Solana-Unreal-SDK/assets/24855008/e4bac429-0218-4275-be27-afab4ec6025f)

Update the request IDs and Methods list ENUM blueprints in the plugin blueprints folder to match the request ids in your new wrapper.
![image](https://github.com/Bifrost-Technologies/Solana-Unreal-SDK/assets/24855008/e626056c-2733-4b0f-99a2-1c75000618c9)
![image](https://github.com/Bifrost-Technologies/Solana-Unreal-SDK/assets/24855008/6e52eca1-8e6a-45da-9b70-f653100b2e14)




### REST API Global Server Auth - Advanced Usage
This is for usage with the Unreal Solana SDK REST API found [here]()
The game template c++ classes come with http request samples to interact with the REST API and allow a game server to request signing from the players without needing to run Unreal SOLNET on the game client. This method is required for Mobile, HTML5, Consoles, & the holo-lens 2. It enables secure solana mobile apps and dedicated server related transaction authentication.

C# API template code
![image](https://github.com/Bifrost-Technologies/Solana-Unreal-SDK/assets/24855008/3c25811b-0483-422f-98be-efb604f10ce8)

C++ GameMode template code
![image](https://github.com/Bifrost-Technologies/Solana-Unreal-SDK/assets/24855008/d208c5fb-4c99-485c-9e9d-b0bab5f4249e)

These are starting points for a dedicated global server with Solana integration. For building fully on-chain games it will require Unreal SOLNET & a linkstream wrapper
