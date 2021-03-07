ifndef SourceRoot

CURRENT :=
CURRENT += CommonFramework/CRC32.cpp
CURRENT += CommonFramework/FrameworkSettings.cpp
CURRENT += CommonPokemon/PokemonSettings.cpp
CURRENT += Libraries/Logging.cpp
CURRENT += Libraries/MessageConverter.cpp
CURRENT += Libraries/PushButtons.cpp
CURRENT += Libraries/PokemonRoutines.cpp
CURRENT += Libraries/Utilities.cpp
CURRENT += Connection/Unicode.cpp
CURRENT += Connection/PABotBaseConnection.cpp
CURRENT += Connection/PABotBase.cpp
CURRENT += Programs/DeviceLogger.cpp
CURRENT += Programs/TurboA.cpp
CURRENT += Programs/ClothingBuyer.cpp
CURRENT += Programs/BallThrower.cpp
CURRENT += Programs/BeamReset.cpp
CURRENT += Programs/FriendDelete.cpp
CURRENT += Programs/DateSpam-WattFarmer.cpp

SOURCES := $(SOURCES) $(addprefix $(CURRENT_DIR)/, $(CURRENT))
endif
