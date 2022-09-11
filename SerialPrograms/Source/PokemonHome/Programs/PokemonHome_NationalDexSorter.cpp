/*  Pokémon Home National Dex Sorter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <iostream>
#include "qglobal.h"
#include "PokemonHome_PageSwap.h"
#include "Common/Cpp/Exceptions.h"
#include "Pokemon/Pokemon_Strings.h"
#include "CommonFramework/Language.h"
#include "PokemonHome_NationalDexSorter.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonLA/Programs/General/PokemonLA_SkipToFullMoon.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "PokemonLA/Inference/PokemonLA_ItemCompatibilityDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


    using std::cout;
    using std::endl;
    using namespace PokemonAutomation::Pokemon;

    NationalDexSorter_Descriptor::NationalDexSorter_Descriptor()
        : SingleSwitchProgramDescriptor(
            "PokemonHome:NationalDexSorter",
            STRING_POKEMON + " Home", STRING_POKEMON + " Home: National Dex Sorter",
            "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/PokemonHome-PageSwap.md",
            "Sort available " + STRING_POKEMON + " according to the National Dex number.",
            FeedbackType::REQUIRED, true,
            PABotBaseLevel::PABOTBASE_12KB
        )
    {}



    NationalDexSorter::NationalDexSorter()
        : START_FROM_DEX_NO(
                  "<b>Pokédex # of first Pokémon to sort:</b>",
                  1
        )
        , GET_TO_DEX_NO(
            "<b>Pokédex # of last Pokémon to sort:</b>",
            905
        )
        , START_FROM_PAGE(
            "<b>Box Page to use for the sorted Pokémon:</b>",
            5
        )
        , CHOOSE_SHINY_FIRST(
            "<b>Look for shiny Pokémon first:</b>",
            true
        )
    {
        PA_ADD_OPTION(START_LOCATION);
        PA_ADD_OPTION(START_FROM_DEX_NO);
        PA_ADD_OPTION(GET_TO_DEX_NO);
        PA_ADD_OPTION(START_FROM_PAGE);
        PA_ADD_OPTION(CHOOSE_SHINY_FIRST);
    }

    int descriptorStartDexNo = 1;
    int descriptorEndDexNo = 905;
    int descriptorStartPage = 5;       // first box to populate
    const int leSpeed = 11;                 // ticks to wait after most of the button presses, seems like this is good enough for most presses.
    const double lineWidth = 0.024;         // how big is the width of the line we're going to check (it works but I have no idea how big this actually is)
    const uint lineCol1 =0xf44336;          // the red to check for in the name bar
    const uint lineCol2 =0xf44336;          // the red to check for in the name bar
    int eCount = 0;                         // if at any point you make too many moves before getting to the next step give up on the pokémon
    const int eCountMax = 35;               // this is the max steps I allow before giving up on a given task

    PokemonLA::ItemCompatibility detect_item_compatibility(const ImageViewRGB32& screen,const ImageFloatBox& box,const uint& col1,const uint& col2,const int& superCheck){
    // This is copied from the SkipToFullMoon code, I use it as a solid color check.
    // I use it in various ways in order to reach my goals, not all of them make sense, but all work.

    const bool replace_background = true;
    ImageRGB32 region = filter_rgb32_range(
        extract_box_reference(screen, box),
        col1, col2, Color(0), replace_background
    );

    ImageStats stats = image_stats(region);
    // std::cout << "Compability color " << stats.average.r << " " << stats.average.g << " " << stats.average.b << std::endl;
    if (stats.average.r > stats.average.b + 50.0){
        return PokemonLA::ItemCompatibility::COMPATIBLE;
    }
    if (stats.average.b > stats.average.r + 50.0){
        return PokemonLA::ItemCompatibility::INCOMPATIBLE;
    }
    //I created these 2 checks for more specific colors, no idea how it works, just know it works :P
    if (stats.average.b > 254.5 && stats.average.r > 252.5 && superCheck == 1){
        return PokemonLA::ItemCompatibility::COMPATIBLE;
    }
    if (stats.average.r >  250 && superCheck == 2){
        return PokemonLA::ItemCompatibility::COMPATIBLE;
    }
    return PokemonLA::ItemCompatibility::NONE;
}

int read_number(Logger& logger, const ImageViewRGB32& image2,const ImageFloatBox& box){
    ImageViewRGB32 image = extract_box_reference(image2, box);
    std::string ocr_text = OCR::ocr_read(Language::English, image);
    std::string normalized;
    bool has_digit = false;
    for (char ch : ocr_text){
        //  4 is commonly misread as A.
        if (ch == 'a' || ch == 'A'){
            normalized += '4';
            has_digit = true;
        }
        if ('0' <= ch && ch <= '9'){
            normalized += ch;
            has_digit = true;
        }
    }

    if (!has_digit){
        return -1;
    }

    int number = std::atoi(normalized.c_str());

    std::string str;
    for (char ch : ocr_text){
        if (ch != '\r' && ch != '\n'){
            str += ch;
        }
    }
    logger.log("OCR Text: \"" + str + "\" -> \"" + normalized + "\" -> Box Page #" + std::to_string(number),COLOR_RED);

    return number;
}

int GetPokemonX(SingleSwitchProgramEnvironment& env, BotBaseContext& context)
{
    pbf_wait(context,15);
    context.wait_for_all_requests();
    eCount = 0;
    int startPositionX = 0;
    // this while tries to find the red bar under the name of the selected pokémon in the first row and returns the column where the bar is found.
    // if it doesn't find it tries to push the pokemon up one row, and it also checks for the "Box Spaces" or "Newest 30" button highlight.
    while(startPositionX == 0){
        if (!(detect_item_compatibility(env.console.video().snapshot(),ImageFloatBox (.065, .139, lineWidth, 0.005),lineCol1,lineCol2,0) != PokemonLA::ItemCompatibility::COMPATIBLE)){
                startPositionX = 1;
                break;
        }
        else if (!(detect_item_compatibility(env.console.video().snapshot(),ImageFloatBox (.09, .139, lineWidth, 0.005),lineCol1,lineCol2,0) != PokemonLA::ItemCompatibility::COMPATIBLE)){
            startPositionX = 2;
            break;
        }
        else if (!(detect_item_compatibility(env.console.video().snapshot(),ImageFloatBox (.115, .139, lineWidth, 0.005),lineCol1,lineCol2,0) != PokemonLA::ItemCompatibility::COMPATIBLE)){
            startPositionX = 3;
            break;
        }
        else if (!(detect_item_compatibility(env.console.video().snapshot(),ImageFloatBox (.195, .139, lineWidth, 0.005),lineCol1,lineCol2,0) != PokemonLA::ItemCompatibility::COMPATIBLE)){
            startPositionX = 4;
            break;
        }
        else if (!(detect_item_compatibility(env.console.video().snapshot(),ImageFloatBox (.270, .139, lineWidth, 0.005),lineCol1,lineCol2,0) != PokemonLA::ItemCompatibility::COMPATIBLE)){
            startPositionX = 5;
            break;
        }
        else if (!(detect_item_compatibility(env.console.video().snapshot(),ImageFloatBox (.333, .139, lineWidth, 0.005),lineCol1,lineCol2,0) != PokemonLA::ItemCompatibility::COMPATIBLE)){
            startPositionX = 6;
            break;
        }
        else if(startPositionX == 0){
            pbf_press_dpad(context, DPAD_UP, 5, 25);
            context.wait_for_all_requests();
            if(!(detect_item_compatibility(env.console.video().snapshot(),ImageFloatBox (.456, .7245, 0.011, 0.038),0xffff4600,0xffe3d26c,0) != PokemonLA::ItemCompatibility::COMPATIBLE)){
                pbf_press_dpad(context, DPAD_RIGHT, 5, leSpeed);
                startPositionX = 7;
                break;
            }
            else if(!(detect_item_compatibility(env.console.video().snapshot(),ImageFloatBox (.276, .7245, 0.011, 0.038),0xffff4600,0xffe3d26c,0) != PokemonLA::ItemCompatibility::COMPATIBLE)){
                startPositionX = 7;
                break;
            }
        }
        eCount ++;
        if (eCount > 7){
            return 0;
        }
        context.wait_for_all_requests();
    }
    if(startPositionX>6){
        env.console.log("Found orange button",COLOR_RED);
    }
    else{
        env.console.log("Found cursor at column #" + std::to_string(startPositionX),COLOR_RED);
    }
    return startPositionX;
}

bool FindBoxSpaces(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    env.console.log("Sorting...",COLOR_RED);
    int pokemonX = GetPokemonX(env, context);
    if (pokemonX == 0){
        return false;
    }
    else if(pokemonX <= 4)
    {
        pbf_press_dpad(context, DPAD_UP, 5, leSpeed);
        pbf_press_dpad(context, DPAD_UP, 5, 30);
    }
    else if(pokemonX <= 6)
    {
        pbf_press_dpad(context, DPAD_UP, 5, leSpeed);
        pbf_press_dpad(context, DPAD_UP, 5, leSpeed);
        pbf_press_dpad(context, DPAD_RIGHT, 5, 30);
    }
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_A, 5, 50);
    pokemonX = GetPokemonX(env, context);
    int i = 0;
    if (pokemonX == 0){
        return false;
    }
    else if(pokemonX<4)
    {
        for(i=1;i<pokemonX;i++){
            pbf_press_dpad(context, DPAD_LEFT, 5, leSpeed);
        }
    }
    else
    {
        for(i=6;i>=pokemonX;i--){
            pbf_press_dpad(context, DPAD_RIGHT, 5, leSpeed);
        }
    }
    return true;                //this position is the first box in the first row

}

void NavigateMenuFast(int itemNo, int totalItems, int midPosition, BotBaseContext& context, SingleSwitchProgramEnvironment& env, int itemOffset = 0)
{
     //this function navigates the menus as fast as possible with the least button presses
    int i = 0;
    if (itemNo < midPosition){
        while(i<itemNo+itemOffset){
            context.wait_for_all_requests();
            if(itemNo-i>=7-itemOffset&&itemNo>=7)
            {
                pbf_press_dpad(context, DPAD_RIGHT, 5, leSpeed);
                i+=7;
            }
            else if(itemNo-i==6-itemOffset&&itemNo>=6)
            {
                pbf_press_dpad(context, DPAD_RIGHT, 5, leSpeed);
                pbf_press_dpad(context, DPAD_UP, 5, leSpeed);
                i+=6;
            }
            else if(itemNo-i==5-itemOffset&&itemNo>=5)
            {
                pbf_press_dpad(context, DPAD_RIGHT, 5, leSpeed);
                pbf_press_dpad(context, DPAD_UP, 5, leSpeed);
                pbf_press_dpad(context, DPAD_UP, 5, leSpeed);
                i+=5;
            }
            else
            {
                pbf_press_dpad(context, DPAD_DOWN, 5, 13);
                i++;
            }
        }
    }
    else{
        itemNo = totalItems-itemNo;
        if(itemNo < 0){itemNo *= -1;}
        pbf_press_dpad(context, DPAD_UP, 5, leSpeed);
        i++;
        while(i<itemNo){
            context.wait_for_all_requests();
            if(itemNo-i>=7&&itemNo>=7)
            {
                pbf_press_dpad(context, DPAD_LEFT, 5, leSpeed);
                i+=7;
            }
            else if(itemNo-i==6&&itemNo>=6)
            {
                pbf_press_dpad(context, DPAD_LEFT, 5, leSpeed);
                pbf_press_dpad(context, DPAD_DOWN, 5, leSpeed);
                i+=6;
            }
            else if(itemNo-i==5&&itemNo>=5)
            {
                pbf_press_dpad(context, DPAD_LEFT, 5, leSpeed);
                pbf_press_dpad(context, DPAD_DOWN, 5, leSpeed);
                pbf_press_dpad(context, DPAD_DOWN, 5, leSpeed);
                i+=5;
            }
            else
            {
                pbf_press_dpad(context, DPAD_UP, 5, 13);
                i++;
            }
        }
    }

    pbf_wait(context, 10);
    if(i>13){
        pbf_wait(context, 35); // if there were too many moves the program will get a little dizzy, let's give it time.
    }
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_A, 5, 50);
    context.wait_for_all_requests();
}

void PokemonHomeSort(int pokedexNo,int letterNo,int movesDown,int movesUp,int upOrDown,int getShiny,int startBoxPage,int startBoxNo,BotBaseContext& context,SingleSwitchProgramEnvironment& env,int type1 = 0, int type2 = 0){
    /* PokemonHomeSort is the main function here.
     *
     * Variables:
     * pokedexNo: is the national pokédex number, it's used to determine the position in the target box of the pokémon
     * letterNo: is the number of the letter in alphabetical order, it's useful to find the pokémon from the searchbar
     * movesDown: it determines how many steps down from the starting point of the letter you can find the pokémon (example once you enter the R for Rattata you need to go down 7 steps to find it in the list
     * movesUp: same as movesDown
     * upOrDown: selects which variable between movesDown and movesUp to use and which function once in the letter menu
     * getShiny: if it's true it tries to find shiny pokémon in your pokémon home, if it's false it only gets the first available pokémon shiny or not.
     * startBoxPage: is the page where the first box (pokémon between 1-30) will be, you can select it but I would make it so you can't select page 6 or 7 (don't have enough boxes for all regional and other forms)
     * startBoxNo: same as above only choses which box to start from in the page (1 to 30)
     * context and env: are necessary variables for all arduino actions, I pass them down to this function
     * type1 and type2: they are 0 by default but if you set them as a number between 1 and 18 it will select pokémon with this types, useful to find regional variants, both need to be set to work correctly
     * (example to find Kanto Raichu you should set both type1 and type2 as electric in order to only find pure electric Raichu.     *
     *
     */
    if((pokedexNo<descriptorStartDexNo)||(pokedexNo>descriptorEndDexNo)){
        return;
    }
    env.console.log("Looking for Pokémon #" + std::to_string(pokedexNo),COLOR_RED);

    int i = 0;

    pbf_mash_button(context, BUTTON_B, 25);                         //get back to basic pokémon selection (good starting point)
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_X, 5, 100);
    pbf_press_button(context, BUTTON_X, 5, 50);
    pbf_press_button(context, BUTTON_A, 5, 50);                    //get to search pokémon by name
    context.wait_for_all_requests();

    env.console.log("Looking for correct first letter.",COLOR_RED);

    NavigateMenuFast(letterNo, 27, 14,context,env, 2);              //this function navigates the menus as fast as possible, here it navigates the alphabet menu

    env.console.log("Looking for correct Pokémon name.",COLOR_RED);

    if (upOrDown == 0){
        NavigateMenuFast(movesUp, 0, movesUp-1,context,env);        // if it's faster to move up in the pokemon list this function activates
    }
    else{
        NavigateMenuFast(movesDown, 0, movesDown+1,context,env);    // if it's faster to move down in the pokemon list this function activates
    }

    if(type1 > 0){                                                      //if type1 is more than 0 it means we're looking for a regional/original variant by finding pokèmon only with the correct type, basculin is not found this way
        env.console.log("Looking for correct Pokémon variant.",COLOR_RED);
        pbf_press_dpad(context, DPAD_DOWN, 5, leSpeed);
        pbf_press_dpad(context, DPAD_DOWN, 5, leSpeed);
        context.wait_for_all_requests();
        pbf_press_button(context, BUTTON_A, 5, 50);

        NavigateMenuFast(type1, 19, 10,context,env);

        pbf_press_dpad(context, DPAD_DOWN, 5, 25);
        context.wait_for_all_requests();
        pbf_press_button(context, BUTTON_A, 5, 50);

        NavigateMenuFast(type2, 19, 10,context,env);

        pbf_press_dpad(context, DPAD_LEFT, 5, leSpeed);
        pbf_press_dpad(context, DPAD_LEFT, 5, 50);
    }

    if(getShiny){                                           //if getShiny flag is 1 try to find shinies and if it fails get nonShiny. if value is 2 it will soon make it so this part of the code only selects available shinies
        env.console.log("Looking for shiny form.",COLOR_RED);
        pbf_press_dpad(context, DPAD_UP, 5, leSpeed);
        context.wait_for_all_requests();
        pbf_press_dpad(context, DPAD_LEFT, 5, leSpeed);
        context.wait_for_all_requests();
        pbf_press_button(context, BUTTON_A, 5, leSpeed);
        pbf_press_dpad(context, DPAD_DOWN, 5, 125);
        context.wait_for_all_requests();
        if(detect_item_compatibility(env.console.video().snapshot(),ImageFloatBox (.059, .157, 0.009, 0.005),0xff000000,0xff000000,2) != PokemonLA::ItemCompatibility::COMPATIBLE){
            pbf_press_dpad(context, DPAD_UP, 5, leSpeed);
            context.wait_for_all_requests();
            pbf_press_button(context, BUTTON_B, 5, 25);
        }
        else{
            context.wait_for_all_requests();
            pbf_press_button(context, BUTTON_A, 5, leSpeed);
            context.wait_for_all_requests();
            pbf_press_button(context, BUTTON_B, 5, 25);
            env.console.log("Found shiny Pokémon #" + std::to_string(pokedexNo),COLOR_RED);
        }
    }
    else{
        pbf_wait(context, 15);
    }
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_B, 5, 50);
    context.wait_for_all_requests();
    if(detect_item_compatibility(env.console.video().snapshot(),ImageFloatBox (.059, .157, 0.009, 0.005),0xff000000,0xff000000,2) != PokemonLA::ItemCompatibility::COMPATIBLE){    //first visual check, code goes on only if it can see a selection arrow
        env.console.log("No Pokémon #" + std::to_string(pokedexNo) + " found!",COLOR_RED);
        pbf_mash_button(context, BUTTON_B, 35);
        return;                  //if there's no pokémon with this name in your pokémon home jump to next pokémon
    }

    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_A, 5, 75);
    pbf_press_button(context, BUTTON_A, 5, 75);
    pbf_press_button(context, BUTTON_Y, 5, 20);                //pick up pokémon and start moving it around
    context.wait_for_all_requests();
    bool goOn = FindBoxSpaces(env,context);     //FindBoxSpaces is all about getting the selected pokèmon to a known position in the pokemon boxes menu. From there the code will take care of where to put it.
    if(!goOn){
        env.console.log("I cannot find the cursor in this page, giving up on Pokémon #" + std::to_string(pokedexNo),COLOR_RED);
        pbf_mash_button(context, BUTTON_B, 35);
        return;
    }
    pbf_wait(context, 35);

    context.wait_for_all_requests();

    int boxPage = read_number(env.console,env.console.video().snapshot(),ImageFloatBox (.269, .1083, 0.022, 0.06));                            //recognizes the number of the page
    if((boxPage < 0)||(boxPage > 7)){
        env.console.log("I cannot read the box page, giving up on Pokémon #" + std::to_string(pokedexNo),COLOR_RED);
        pbf_mash_button(context, BUTTON_B, 35);
        return;
    }
    int boxDistance = 0;
    if(boxPage > startBoxPage)
    {
        boxDistance = boxPage-startBoxPage;
        if (boxDistance > 3){
            for(i=0;i<7-boxDistance;i++){
                pbf_press_button(context, BUTTON_R, 10, 65);
            }
        }
        else{
            for(i=0;i<boxDistance;i++){
                pbf_press_button(context, BUTTON_L, 10, 65);
            }
        }
    }
    else if(startBoxPage > boxPage)
    {
        boxDistance = startBoxPage-boxPage;
        if (boxDistance > 3){
            for(i=0;i<7-boxDistance;i++){
                pbf_press_button(context, BUTTON_L, 10, 65);
            }
        }
        else{
            for(i=0;i<boxDistance;i++){
                pbf_press_button(context, BUTTON_R, 10, 65);
            }
        }
    }

    env.console.log("More sorting...",COLOR_RED);
    int targetBoxPage = 1;                      //all this code down here reliably calculates which box and then from inside the box which position the pokémon will move to STARTING FROM THE FIRST ROW AND FIRST COLUMN
    int targetPos = pokedexNo;
    int targetX = 1;
    int targetY = 1;
    int targetBoxRow = 1;
    int targetBoxColumn = 1;
    while(targetPos>180){
        targetPos -= 180;
        targetBoxRow++;
    }
    while(targetBoxRow>5){
        targetBoxRow -=5;
        targetBoxPage ++;
    }
    while(targetPos>30){
        targetPos -= 30;
        targetBoxColumn++;
    }

    int nowPage = 1;

    while(nowPage<targetBoxPage){
        pbf_press_button(context, BUTTON_R, 10, 65);
        nowPage++;
        context.wait_for_all_requests();
    }
    if(targetBoxColumn<4){
        for(i=1;i<targetBoxColumn;i++){
            pbf_press_dpad(context, DPAD_RIGHT, 5, leSpeed);
        }
    }
    else{
        for(i=6;i>=targetBoxColumn;i--){
            pbf_press_dpad(context, DPAD_LEFT, 5, leSpeed);
        }

    }
    if(targetBoxRow<5){
        for(i=1;i<targetBoxRow;i++){
            pbf_press_dpad(context, DPAD_DOWN, 5, leSpeed);
        }
    }
    else{
            pbf_press_dpad(context, DPAD_UP, 5, leSpeed);
            pbf_press_dpad(context, DPAD_UP, 5, leSpeed);
            pbf_press_dpad(context, DPAD_UP, 5, leSpeed);
    }
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_A, 5, 100);
    while(targetPos>6){
        targetPos -= 6;
        targetY++;
    }
    targetX = targetPos;
    if(targetX<4){
        for(i=1;i<targetX;i++){
            pbf_press_dpad(context, DPAD_RIGHT, 5, leSpeed);
        }
    }
    else{
        for(i=6;i>=targetX;i--){
            pbf_press_dpad(context, DPAD_LEFT, 5, leSpeed);
        }

    }
    if(targetY<5){
        for(i=1;i<targetY;i++){
            pbf_press_dpad(context, DPAD_DOWN, 5, leSpeed);
        }
    }
    else{
            pbf_press_dpad(context, DPAD_UP, 5, leSpeed);
            pbf_press_dpad(context, DPAD_UP, 5, leSpeed);
            pbf_press_dpad(context, DPAD_UP, 5, leSpeed);
    }

    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_A, 5, leSpeed);         //done gg
    env.console.log("Pokémon #" + std::to_string(pokedexNo)+" Successfully Sorted!",COLOR_RED);

}

void NationalDexSorter::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    descriptorStartDexNo = START_FROM_DEX_NO;
    descriptorEndDexNo = GET_TO_DEX_NO;
    descriptorStartPage = START_FROM_PAGE;

    if ((descriptorStartPage > 6)||(descriptorStartPage < 1))
    {
        env.console.log("Impossible start box page. Will start from page 5.",COLOR_RED);
        descriptorStartPage = 5;
    }
    if ((descriptorStartDexNo > 905)||(descriptorStartDexNo < 1))
    {
        env.console.log("Impossible Pokédex number. Program will terminate.",COLOR_RED);
        return;
    }
    if ((descriptorEndDexNo > 905)||(descriptorEndDexNo < 1))
    {
        env.console.log("Impossible Pokédex number. Program will terminate.",COLOR_RED);
        return;
    }
    if (descriptorEndDexNo < descriptorStartDexNo)
    {
        env.console.log("Impossible Pokédex range. Program will terminate.",COLOR_RED);
        return;
    }

    pbf_mash_button(context, BUTTON_B, 2.5 * TICKS_PER_SECOND);
    PokemonHomeSort(1,2,43,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);      //I have a database with the needed data to populate the function to move correctly all 905 pokémon, will move to json whenever I understand it
    PokemonHomeSort(2,9,8,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(3,22,7,16,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(4,3,19,57,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(5,3,20,56,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(6,3,17,59,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(7,19,94,30,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(8,23,3,22,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(9,2,24,25,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(10,3,11,65,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(11,13,41,32,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(12,2,47,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(13,23,6,19,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(14,11,3,25,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(15,2,12,37,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(16,16,25,34,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(17,16,24,35,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(18,16,23,36,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(19,18,7,28,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,1,1);
    PokemonHomeSort(20,18,6,29,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,1,1);
    PokemonHomeSort(21,19,85,39,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(22,6,2,28,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(23,5,4,21,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(24,1,20,20,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(25,16,28,31,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(26,18,1,34,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,13,13);
    PokemonHomeSort(27,19,7,117,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,5,5);
    PokemonHomeSort(28,19,8,116,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,5,5);
    PokemonHomeSort(29,14,6,14,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(30,14,8,12,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(31,14,5,15,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(32,14,7,13,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(33,14,9,11,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(34,14,4,16,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(35,3,39,37,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(36,3,38,38,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(37,22,22,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,10,10);
    PokemonHomeSort(38,14,12,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,10,10);
    PokemonHomeSort(39,10,2,6,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(40,23,13,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(41,26,11,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(42,7,23,34,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(43,15,2,9,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(44,7,21,36,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(45,22,14,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(46,16,9,50,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(47,16,10,49,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(48,22,6,17,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(49,22,5,18,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(50,4,19,37,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,5,5);
    PokemonHomeSort(51,4,46,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,5,5);
    PokemonHomeSort(52,13,37,36,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,1,1);
    PokemonHomeSort(53,16,16,43,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,1,1);
    PokemonHomeSort(54,16,52,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(55,7,25,32,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(56,13,18,55,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(57,16,49,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(58,7,49,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,10,10);
    PokemonHomeSort(59,1,21,19,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,10,10);
    PokemonHomeSort(60,16,38,21,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(61,16,39,20,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(62,16,40,19,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(63,1,1,39,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(64,11,2,26,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(65,1,8,32,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(66,13,2,71,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(67,13,1,72,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(68,13,0,73,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(69,2,16,33,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(70,23,7,18,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(71,22,11,12,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(72,20,10,43,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(73,20,11,42,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(74,7,10,47,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,6,5);
    PokemonHomeSort(75,7,40,17,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,6,5);
    PokemonHomeSort(76,7,26,31,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,6,5);
    PokemonHomeSort(77,16,42,17,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,10,10);
    PokemonHomeSort(78,18,5,30,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,10,10);
    PokemonHomeSort(79,19,68,56,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,11,14);
    PokemonHomeSort(80,19,66,58,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,11,14);
    PokemonHomeSort(81,13,9,64,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(82,13,10,63,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(83,6,1,29,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(84,4,22,34,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(85,4,21,35,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(86,19,24,100,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(87,4,12,44,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(88,7,43,14,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,4,4);
    PokemonHomeSort(89,13,68,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,4,4);
    PokemonHomeSort(90,19,35,89,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(91,3,42,34,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(92,7,6,51,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(93,8,6,26,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(94,7,9,48,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(95,15,5,6,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(96,4,42,14,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(97,8,31,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(98,11,20,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(99,11,10,18,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(100,22,20,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,13,13);
    PokemonHomeSort(101,5,9,16,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,13,13);
    PokemonHomeSort(102,5,22,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(103,5,23,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,12,14);
    PokemonHomeSort(104,3,71,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(105,13,25,48,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,5,5);
    PokemonHomeSort(106,8,18,14,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(107,8,17,15,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(108,12,14,24,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(109,11,17,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(110,23,8,17,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,4,4);
    PokemonHomeSort(111,18,20,15,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(112,18,19,16,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(113,3,16,60,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(114,20,2,51,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(115,11,4,24,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(116,8,26,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(117,19,20,104,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(118,7,24,33,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(119,19,21,103,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(120,19,101,23,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(121,19,100,24,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(122,13,63,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,14,18);
    PokemonHomeSort(123,19,19,105,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(124,10,7,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(125,5,6,19,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(126,13,7,66,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(127,16,33,26,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(128,20,8,45,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(129,13,6,67,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(130,7,56,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(131,12,4,34,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(132,4,20,36,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(133,5,2,23,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(134,22,3,20,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(135,10,4,4,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(136,6,12,18,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(137,16,45,14,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(138,15,3,8,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(139,15,4,7,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(140,11,0,28,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(141,11,1,27,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(142,1,5,35,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(143,19,77,47,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(144,1,32,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,15,3);
    PokemonHomeSort(145,26,3,11,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,13,3);
    PokemonHomeSort(146,13,57,16,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,10,3);
    PokemonHomeSort(147,4,35,21,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(148,4,30,26,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(149,4,31,25,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(150,13,43,30,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(151,13,42,31,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(152,3,27,49,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(153,2,9,40,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(154,13,32,41,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(155,3,75,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(156,17,1,3,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(157,20,48,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(158,20,29,24,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(159,3,67,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(160,6,5,25,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(161,19,26,98,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(162,6,29,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(163,8,24,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(164,14,14,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(165,12,12,26,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(166,12,11,27,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(167,19,89,35,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(168,1,27,13,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(169,3,66,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(170,3,30,46,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(171,12,3,35,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(172,16,22,37,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(173,3,40,36,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(174,9,0,9,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(175,20,22,31,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(176,20,23,30,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(177,14,1,19,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(178,24,0,3,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(179,13,23,50,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(180,6,9,21,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(181,1,15,25,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(182,2,15,34,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(183,13,24,49,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(184,1,38,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(185,19,109,15,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(186,16,37,22,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(187,8,25,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(188,19,57,67,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(189,10,6,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(190,1,7,33,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(191,19,112,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(192,19,111,13,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(193,25,2,4,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(194,23,20,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(195,17,0,4,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(196,5,18,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(197,21,0,7,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(198,13,71,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(199,19,67,57,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,11,14);
    PokemonHomeSort(200,13,55,18,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(201,21,2,5,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(202,23,17,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(203,7,13,44,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(204,16,32,27,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(205,6,21,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(206,4,47,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(207,7,19,38,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(208,19,102,22,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(209,19,80,44,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(210,7,38,19,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(211,17,3,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,11,4);
    PokemonHomeSort(212,19,14,110,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(213,19,43,81,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(214,8,13,19,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(215,19,73,51,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,17,15);
    PokemonHomeSort(216,20,9,44,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(217,21,4,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(218,19,69,55,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(219,13,4,69,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(220,19,120,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(221,16,30,29,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(222,3,52,24,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,11,6);
    PokemonHomeSort(223,18,16,19,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(224,15,1,10,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(225,4,9,47,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(226,13,19,54,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(227,19,55,69,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(228,8,28,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(229,8,27,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(230,11,9,19,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(231,16,18,41,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(232,4,23,33,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(233,16,47,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(234,19,96,28,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(235,19,71,53,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(236,20,51,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(237,8,19,13,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(238,19,72,52,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(239,5,10,15,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(240,13,3,70,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(241,13,49,24,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(242,2,27,22,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(243,18,2,33,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(244,5,16,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(245,19,110,14,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(246,12,6,32,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(247,16,54,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(248,20,49,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(249,12,29,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(250,8,20,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(251,3,12,64,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(252,20,37,16,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(253,7,48,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(254,19,13,111,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(255,20,24,29,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(256,3,47,29,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(257,2,25,24,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(258,13,66,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(259,13,27,46,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(260,19,117,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(261,16,43,16,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(262,13,46,27,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(263,26,8,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,1,1);
    PokemonHomeSort(264,12,19,19,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,1,1);
    PokemonHomeSort(265,23,22,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(266,19,46,78,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(267,2,11,38,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(268,3,9,67,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(269,4,54,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(270,12,25,13,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(271,12,23,15,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(272,12,28,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(273,19,23,101,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(274,14,19,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(275,19,39,85,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(276,20,0,53,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(277,19,119,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(278,23,15,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(279,16,14,45,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(280,18,3,32,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(281,11,11,17,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(282,7,5,52,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(283,19,113,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(284,13,28,45,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(285,19,42,82,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(286,2,36,13,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(287,19,64,60,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(288,22,12,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(289,19,63,61,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(290,14,11,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(291,14,13,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(292,19,33,91,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(293,23,12,13,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(294,12,26,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(295,5,24,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(296,13,12,61,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(297,8,2,30,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(298,1,39,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(299,14,17,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(300,19,58,66,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(301,4,8,48,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(302,19,0,124,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(303,13,29,44,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(304,1,30,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(305,12,0,38,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(306,1,6,34,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(307,13,31,42,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(308,13,30,43,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(309,5,8,17,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(310,13,17,56,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(311,16,35,24,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(312,13,54,19,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(313,22,17,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(314,9,1,8,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(315,18,29,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(316,7,52,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(317,19,116,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(318,3,8,68,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(319,19,31,93,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(320,23,0,25,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(321,23,1,24,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(322,14,18,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(323,3,3,73,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(324,20,25,28,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(325,19,92,32,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(326,7,51,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(327,19,90,34,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(328,20,36,17,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(329,22,9,14,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(330,6,18,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(331,3,0,76,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(332,3,1,75,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(333,19,114,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(334,1,11,29,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(335,26,2,12,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(336,19,29,95,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(337,12,32,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(338,19,84,40,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(339,2,4,45,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(340,23,11,14,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(341,3,51,25,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(342,3,63,13,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(343,2,1,48,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(344,3,37,39,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(345,12,16,22,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(346,3,60,16,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(347,1,16,24,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(348,1,28,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(349,6,3,27,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(350,13,48,25,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(351,3,10,66,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(352,11,7,21,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(353,19,44,80,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(354,2,2,47,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(355,4,53,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(356,4,51,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(357,20,39,14,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(358,3,29,47,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(359,1,2,38,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(360,23,23,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(361,19,78,46,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(362,7,16,41,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(363,19,88,36,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(364,19,22,102,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(365,23,2,23,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(366,3,34,42,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(367,8,29,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(368,7,32,25,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(369,18,15,20,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(370,12,34,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(371,2,0,49,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(372,19,34,90,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(373,19,1,123,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(374,2,14,35,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(375,13,40,33,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(376,13,39,34,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(377,18,13,22,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(378,18,9,26,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(379,18,14,21,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(380,12,7,31,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(381,12,8,30,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(382,11,26,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(383,7,47,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(384,18,8,27,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(385,10,3,5,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(386,4,11,45,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(387,20,44,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(388,7,46,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(389,20,28,25,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(390,3,28,48,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(391,13,58,15,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(392,9,5,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(393,16,34,25,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(394,16,50,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(395,5,14,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(396,19,99,25,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(397,19,98,26,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(398,19,97,27,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(399,2,20,29,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(400,2,19,30,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(401,11,21,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(402,11,22,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(403,19,41,83,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(404,12,35,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(405,12,36,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(406,2,41,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(407,18,30,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(408,3,62,14,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(409,18,4,31,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(410,19,38,86,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(411,2,8,41,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(412,2,46,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(413,23,21,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(414,13,62,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(415,3,46,30,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(416,22,8,15,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(417,16,0,59,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(418,2,42,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(419,6,15,15,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(420,3,23,53,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(421,3,22,54,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(422,19,36,88,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(423,7,7,50,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(424,1,13,27,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(425,4,39,17,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(426,4,38,18,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(427,2,44,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(428,12,24,14,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(429,13,56,17,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(430,8,21,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(431,7,17,40,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(432,16,56,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(433,3,31,45,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(434,19,108,16,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(435,19,61,63,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(436,2,39,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(437,2,38,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(438,2,31,18,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(439,13,50,23,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(440,8,1,31,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(441,3,21,55,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(442,19,91,33,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(443,7,11,46,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(444,7,0,57,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(445,7,4,53,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(446,13,69,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(447,18,24,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(448,12,27,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(449,8,15,17,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(450,8,16,16,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(451,19,59,65,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(452,4,34,22,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(453,3,65,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(454,20,33,20,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(455,3,6,70,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(456,6,8,22,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(457,12,30,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(458,13,20,53,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(459,19,79,45,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(460,1,0,40,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(461,23,5,20,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(462,13,11,62,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(463,12,13,25,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(464,18,21,14,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(465,20,3,50,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(466,5,7,18,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(467,13,8,65,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(468,20,21,32,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(469,25,3,3,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(470,12,9,29,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(471,7,15,42,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(472,7,20,37,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(473,13,14,59,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(474,16,46,13,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(475,7,1,56,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(476,16,51,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(477,4,52,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(478,6,26,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(479,18,31,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(480,21,6,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(481,13,38,35,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(482,1,37,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(483,4,16,40,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(484,16,1,58,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(485,8,10,22,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(486,18,12,23,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(487,7,14,43,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(488,3,64,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(489,16,21,38,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(490,13,15,58,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(491,4,0,56,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(492,19,32,92,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(493,1,22,18,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(494,22,10,13,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(495,19,75,49,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(496,19,28,96,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(497,19,27,97,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(498,20,12,41,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(499,16,27,32,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(500,5,12,13,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(501,15,9,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(502,4,13,43,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(503,19,4,120,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,11,11);
    PokemonHomeSort(504,16,12,47,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(505,23,4,21,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(506,12,18,20,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(507,8,14,18,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(508,19,105,19,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(509,16,55,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(510,12,15,23,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(511,16,7,52,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(512,19,50,74,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(513,16,8,51,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(514,19,51,73,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(515,16,6,53,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(516,19,49,75,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(517,13,70,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(518,13,72,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(519,16,26,33,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(520,20,35,18,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(521,21,1,6,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(522,2,28,21,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(523,26,5,9,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(524,18,26,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(525,2,29,20,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(526,7,12,45,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(527,23,18,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(528,19,122,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(529,4,40,16,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(530,5,21,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(531,1,33,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(532,20,18,35,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(533,7,54,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(534,3,49,27,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(535,20,45,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(536,16,3,56,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(537,19,25,99,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(538,20,15,38,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(539,19,10,114,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(540,19,30,94,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(541,19,115,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(542,12,10,28,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(543,22,4,19,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(544,23,10,15,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(545,19,15,109,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(546,3,57,19,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(547,23,9,16,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(548,16,17,42,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(549,12,17,21,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(550,2,7,42,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(551,19,6,118,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(552,11,23,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(553,11,24,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(554,4,3,53,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,10,10);
    PokemonHomeSort(555,4,1,55,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,10,10);
    PokemonHomeSort(556,13,21,52,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(557,4,55,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(558,3,68,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(559,19,18,106,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(560,19,17,107,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(561,19,45,79,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(562,25,0,6,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,8,8);
    PokemonHomeSort(563,3,45,31,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(564,20,19,34,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(565,3,7,69,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(566,1,23,17,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(567,1,24,16,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(568,20,40,13,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(569,7,3,54,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(570,26,10,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,17,17);
    PokemonHomeSort(571,26,9,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,17,17);
    PokemonHomeSort(572,13,52,21,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(573,3,32,44,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(574,7,34,23,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(575,7,36,21,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(576,7,35,22,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(577,19,83,41,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(578,4,48,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(579,18,18,17,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(580,4,45,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(581,19,118,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(582,22,1,22,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(583,22,0,23,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(584,22,2,21,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(585,4,6,50,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(586,19,11,113,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(587,5,13,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(588,11,5,23,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(589,5,17,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(590,6,20,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(591,1,14,26,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(592,6,23,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(593,10,1,7,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(594,1,10,30,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(595,10,5,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(596,7,2,55,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(597,6,6,24,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(598,6,7,23,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(599,11,15,13,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(600,11,12,16,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(601,11,16,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(602,20,46,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(603,5,0,25,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(604,5,1,24,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(605,5,11,14,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(606,2,13,36,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(607,12,22,16,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(608,12,1,37,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(609,3,15,61,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(610,1,36,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(611,6,22,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(612,8,8,24,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(613,3,70,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(614,2,10,39,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(615,3,69,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(616,19,37,87,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(617,1,3,37,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(618,19,107,17,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,5,13);
    PokemonHomeSort(619,13,44,29,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(620,13,45,28,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(621,4,43,13,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(622,7,27,30,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(623,7,29,28,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(624,16,13,46,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(625,2,22,27,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(626,2,32,17,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(627,18,33,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(628,2,35,14,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,1,3);
    PokemonHomeSort(629,22,21,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(630,13,16,57,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(631,8,9,23,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(632,4,50,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(633,4,7,49,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(634,26,12,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(635,8,30,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(636,12,5,33,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(637,22,19,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(638,3,44,32,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(639,20,13,40,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(640,22,15,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(641,20,26,27,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(642,20,16,37,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(643,18,17,18,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(644,26,6,8,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(645,12,2,36,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(646,11,27,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(647,11,8,20,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(648,13,34,39,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,1,14);
    PokemonHomeSort(649,7,8,49,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(650,3,25,51,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(651,17,2,2,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(652,3,24,52,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(653,6,4,26,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(654,2,34,15,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(655,4,10,46,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(656,6,24,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(657,6,25,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(658,7,42,15,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(659,2,45,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(660,4,18,38,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(661,6,14,16,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(662,6,13,17,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(663,20,1,52,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(664,19,12,112,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(665,19,87,37,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(666,22,16,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(667,12,20,18,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(668,16,57,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(669,6,10,20,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(670,6,16,14,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(671,6,17,13,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(672,19,56,68,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(673,7,22,35,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(674,16,4,55,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(675,16,5,54,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(676,6,28,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(677,5,19,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(678,13,36,37,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(679,8,22,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(680,4,25,31,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(681,1,4,36,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(682,19,93,31,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(683,1,29,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(684,19,121,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(685,19,70,54,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(686,9,6,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(687,13,13,60,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(688,2,21,28,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(689,2,3,46,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(690,19,60,64,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(691,4,28,28,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(692,3,35,41,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(693,3,36,40,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(694,8,12,20,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(695,8,11,21,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(696,20,52,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(697,20,50,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(698,1,12,28,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(699,1,34,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(700,19,123,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(701,8,7,25,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(702,4,5,51,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(703,3,4,72,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(704,7,31,26,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(705,19,65,59,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(706,7,30,27,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(707,11,14,14,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(708,16,19,40,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(709,20,38,15,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(710,16,53,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(711,7,37,20,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(712,2,17,32,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(713,1,35,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(714,14,15,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(715,14,16,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(716,24,1,2,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(717,25,5,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(718,26,13,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(719,4,17,39,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(720,8,23,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,14,8);
    PokemonHomeSort(721,22,18,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(722,18,32,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(723,4,2,54,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(724,4,4,52,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(725,12,21,17,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(726,20,27,26,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(727,9,3,6,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(728,16,44,15,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(729,2,37,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(730,16,48,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(731,16,29,30,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(732,20,41,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(733,20,30,23,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(734,25,4,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(735,7,53,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(736,7,50,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(737,3,18,58,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(738,22,13,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(739,3,59,17,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(740,3,58,18,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(741,15,8,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(742,3,74,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(743,18,22,13,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(744,18,25,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(745,12,37,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(746,23,16,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(747,13,22,51,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(748,20,31,22,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(749,13,65,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(750,13,67,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(751,4,14,42,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(752,1,19,21,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(753,6,19,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(754,12,33,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(755,13,59,14,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(756,19,40,84,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(757,19,2,122,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(758,19,3,121,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(759,19,106,18,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(760,2,18,31,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(761,2,33,16,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(762,19,103,21,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(763,20,42,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(764,3,48,28,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(765,15,6,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(766,16,11,48,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(767,23,14,11,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(768,7,28,29,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(769,19,9,115,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(770,16,2,57,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(771,16,58,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(772,20,47,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(773,19,48,76,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(774,13,53,20,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(775,11,18,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(776,20,43,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(777,20,20,33,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(778,13,51,22,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(779,2,40,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(780,4,33,23,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(781,4,15,41,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(782,10,0,8,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(783,8,0,32,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(784,11,19,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(785,20,6,47,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(786,20,7,46,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(787,20,4,49,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(788,20,5,48,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(789,3,56,20,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(790,3,55,21,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(791,19,82,42,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(792,12,31,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(793,14,10,10,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(794,2,48,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(795,16,20,39,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(796,24,2,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(797,3,13,63,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(798,11,6,22,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(799,7,55,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(800,14,2,18,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,14,14);
    PokemonHomeSort(801,13,5,68,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(802,13,26,47,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(803,16,36,23,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(804,14,0,20,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(805,19,95,29,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(806,2,23,26,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(807,26,7,7,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(808,13,35,38,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(809,13,33,40,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(810,7,45,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(811,20,17,36,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(812,18,23,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(813,19,16,108,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(814,18,0,35,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(815,3,33,43,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(816,19,81,43,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(817,4,41,15,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(818,9,7,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(819,19,62,62,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(820,7,41,16,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(821,18,28,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(822,3,54,22,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(823,3,53,23,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(824,2,26,23,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(825,4,24,32,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(826,15,7,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(827,14,3,17,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(828,20,14,39,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(829,7,33,24,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(830,5,5,20,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(831,23,19,6,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(832,4,44,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(833,3,26,50,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(834,4,36,20,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(835,25,1,5,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(836,2,30,19,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(837,18,27,8,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(838,3,5,71,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(839,3,43,33,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(840,1,18,22,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(841,6,11,19,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(842,1,17,23,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(843,19,47,77,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(844,19,5,119,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(845,3,61,15,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(846,1,31,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(847,2,5,44,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(848,20,32,21,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(849,20,34,19,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(850,19,54,70,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(851,3,14,62,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(852,3,41,35,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(853,7,39,18,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(854,19,52,72,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(855,16,41,18,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(856,8,3,29,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(857,8,5,27,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(858,8,4,28,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(859,9,2,7,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(860,13,60,13,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(861,7,44,13,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(862,15,0,11,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(863,16,15,44,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(864,3,73,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(865,19,53,71,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(866,13,64,9,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(867,18,34,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(868,13,47,26,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(869,1,9,31,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(870,6,0,30,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(871,16,31,28,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(872,19,76,48,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(873,6,27,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(874,19,104,20,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(875,5,3,22,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(876,9,4,5,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(877,13,61,12,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(878,3,72,4,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(879,3,50,26,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(880,4,27,29,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(881,1,26,14,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(882,4,26,30,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(883,1,25,15,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(884,4,49,7,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(885,4,37,19,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(886,4,32,24,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(887,4,29,27,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(888,26,0,14,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(889,26,1,13,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(890,5,20,5,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(891,11,25,3,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(892,21,5,2,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(893,26,4,10,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(894,18,11,24,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(895,18,10,25,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(896,7,18,39,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(897,19,86,38,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(898,3,2,74,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,14,12);
    PokemonHomeSort(899,23,24,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(900,11,13,15,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(901,21,3,4,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(902,2,6,43,1,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(903,19,74,50,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(904,15,10,1,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env);
    PokemonHomeSort(905,5,15,10,0,CHOOSE_SHINY_FIRST,descriptorStartPage,1,context,env,18,3);



    while(true){
        pbf_press_button(context, BUTTON_B, 5, 10 * TICKS_PER_SECOND);
    }
}




}
}
}
