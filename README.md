# Unreal Engine 5 Solana Template & Toolkit

This project is currently in development for submission to the Solana Grizzlython. Come back soon!
### Do NOT fork or use until after the hackathon is over! Thank you!
 
Developing a complete game template and toolkit for building blockchain games on Solana with Unreal Engine 5.

The toolkit comes with a starting template for Solana games(top down, 3rd person or first person), a multiplayer game server SDK, and REST API Http server to handle global server calls for players.

Using UnrealCLR, we can use [SOL.NET](https://github.com/bmresearch/Solnet) and the power of .NET runtime inside the game giving developers the ability to communicate with the Solana blockchain without having to build an entire c++ library from scratch.

In the MMO demo the gameserver relies heavily on the REST API server which acts as a middle-man between the game server and desktop wallet so no actual transactions are processed or crafted in-game protecting the game from being classified as a financial application/product. This is important because 99% of Ad providers including app stores will not market or list financial applications.

 For specific application types like VR/AR/XR where an in-game wallet would be needed then we use UnrealCLR to call the Solana UE5 SDK directly in the game.
 Using [SolanaMobile Unreal Engine 5 Plugin](https://github.com/Bifrost-Technologies/SolanaMobile-UnrealEngine5-Plugin) or [LinkStream](https://github.com/Bifrost-Technologies/Link-Stream) the game is capable of requesting signing via [Tranquility Wallet](https://github.com/Bifrost-Technologies/Tranquility) and popular mobile wallets through encrypted TCP & Websocket loopback mechanics.

The demo will cover purchasing items in-game inside a virtual store along with showcasing a vending machine style minting station for Metaplex protocol.
Some bonus features if there is enough time will be in-game trading of assets through Openbook DEX
