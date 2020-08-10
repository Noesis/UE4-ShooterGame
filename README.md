# NoesisGUI Unreal Engine ShooterGame Demo

![Demo screenshot](https://noesis.github.io/NoesisGUI/Samples/ShooterGame/UE4/Screenshot.PNG)

## Introduction

This is a modified version of Epic's ShooterGame tutorial in which we've replaced the original UI (HUD and menus) with one made with NoesisGUI. Because the original UI was done using native code (`HUD` and Slate `Widget`s), we've had to make some changes to expose some data and functionality to `Blueprint`s, but all the UI is done using XAML and `Blueprint`s.

## Demo Breakdown

### Blueprint

* `Blueprint'/Game/ShooterGame/ShooterGame_Menu_Noesis.ShooterGame_Menu_Noesis'`, `Blueprint'/Game/ShooterGame/ShooterGame_TeamDeathmatch_Noesis.ShooterGame_TeamDeathmatch_Noesis'` and `Blueprint'/Game/ShooterGame/ShotterGame_FreeForAll_Noesis.ShotterGame_FreeForAll_Noesis'`: These `GameMode`s replace the original ones. They specify which controllers to use (`Blueprint'/Game/ShooterGame/ShooterPlayerController_Menu_Noesis.ShooterPlayerController_Menu_Noesis'` in `Blueprint'/Game/ShooterGame/ShooterGame_Menu_Noesis.ShooterGame_Menu_Noesis'` and `Blueprint'/Game/ShooterGame/ShooterPlayerController_Noesis.ShooterPlayerController_Noesis'` in the other two), and specify no `HUD` will be used, as we will be using NoesisGUI for it.

![Blueprint'/Game/ShooterGame/ShooterGame_Menu_Noesis.ShooterGame_Menu_Noesis'](https://noesis.github.io/NoesisGUI/Samples/ShooterGame/UE4/ShooterGame_Menu_Noesis.PNG)

* `Blueprint'/Game/ShooterGame/ShooterPlayerController_Menu_Noesis.ShooterPlayerController_Menu_Noesis'`: It replaces the `PlayerController` used in the main menu of the game. It binds to the events `OnStateStarted` and `OnStateEnding` from `ShooterGameInstance`. The function `OnStateStarted` is responsible for creating a `Widget` from the `NoesisView` `NoesisBlueprint'/Game/ShooterGame/MainMenuView.MainMenuView'`, adding it to the viewport and setting the input mode.

![Blueprint'/Game/ShooterGame/ShooterPlayerController_Menu_Noesis.ShooterPlayerController_Menu_Noesis'](https://noesis.github.io/NoesisGUI/Samples/ShooterGame/UE4/ShooterPlayerController_Menu_Noesis.PNG)

* `Blueprint'/Game/ShooterGame/ShooterPlayerController_Noesis.ShooterPlayerController_Noesis'`: It replaces the `PlayerController` used in game. It handles the `InGameMenu` `InputAction` to show and hide the in game menu, which is a `Widget` created from the `NoesisView` `NoesisBlueprint'/Game/ShooterGame/GameMenuView.GameMenuView'`.

![Blueprint'/Game/ShooterGame/ShooterPlayerController_Noesis.ShooterPlayerController_Noesis'](https://noesis.github.io/NoesisGUI/Samples/ShooterGame/UE4/ShooterPlayerController_Noesis.PNG)

* `NoesisBlueprint'/Game/ShooterGame/HUDView.HUDView'`: It's the `NoesisView` for the in game HUD. It also the `DataContext` for the `NoesisXaml` `NoesisXaml'/Game/ShooterGame/HUD.HUD'`. It exposes the properties used for `Binding` as variables, which are grouped in the 'Properties' category.

![NoesisBlueprint'/Game/ShooterGame/HUDView.HUDView'](https://noesis.github.io/NoesisGUI/Samples/ShooterGame/UE4/HUDView.PNG)

* `NoesisBlueprint'/Game/ShooterGame/GameMenuView.GameMenuView'`: It's the `NoesisView` for the in game HUD. It also the `DataContext` for the `NoesisXaml` `NoesisXaml'/Game/ShooterGame/GameMenu.GameMenu'`. It exposes the properties used for `Binding` as variables, which are grouped in the 'Properties' category, and a few as pairs of Get/Set functions. Note how the properties `InfiniteAmmo`, `InfiniteClip`, `FreezeMatchTimer` and `HealthRegeneration`, which are the ones implemented as functions and get their data from the `ShooterPlayerController` are manually notified in the `Initialize` function when said variable changes, as well as in the corresponding Set functions. It also implements the `Command`s `ApplyOptions`, `Resume`, `BackToMainMenu` and `Quit`.

![NoesisBlueprint'/Game/ShooterGame/GameMenuView.GameMenuView'](https://noesis.github.io/NoesisGUI/Samples/ShooterGame/UE4/GameMenuView.PNG)

* `NoesisBlueprint'/Game/ShooterGame/MainMenuView.MainMenuView'`: It's the `NoesisView` for the in game HUD. It also the `DataContext` for the `NoesisXaml` `NoesisXaml'/Game/ShooterGame/MainMenu.MainMenu'`. It exposes the properties used for `Binding` as variables, which are grouped in the 'Properties' category. It also implements the `Command`s `ApplyOptions`, `HostFFA`, `HostTDM`, `SearchServer`, `JoinGame` and `Quit`.`SearchServer` and `JoinGame` are used along `CanExecuteSearchServer` and `CanExecuteJoinGame`, that serve as the `CanExecute` functions of their respective `Command`s.

![NoesisBlueprint'/Game/ShooterGame/MainMenuView.MainMenuView'](https://noesis.github.io/NoesisGUI/Samples/ShooterGame/UE4/MainMenuView.PNG)

* `ShooterEntry - Level Blueprint`: It has a `GameMode` override to use `Blueprint'/Game/ShooterGame/ShooterGame_Menu_Noesis.ShooterGame_Menu_Noesis'`.

![ShooterEntry - Level Blueprint](https://noesis.github.io/NoesisGUI/Samples/ShooterGame/UE4/ShooterEntry-LevelBlueprint.PNG)