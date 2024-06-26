#include "main.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "GlobalNamespace/SaberManager.hpp"
#include "GlobalNamespace/SaberModelController.hpp"
#include "GlobalNamespace/MainMenuViewController.hpp"
#include "modules/SaberColorManager.hpp"
#include "ModSettingsViewController.hpp"
#include "bsml/shared/BSML.hpp"


#include <map>

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};

using namespace GlobalNamespace;

Configuration& getConfig() {
    static Configuration config(modInfo);
    return config;
}

bool hasLoaded = false;

MAKE_HOOK_MATCH(MainMenuViewControllerDidActivate, &GlobalNamespace::MainMenuViewController::DidActivate, void, GlobalNamespace::MainMenuViewController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    MainMenuViewControllerDidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    if(firstActivation) {
        hasLoaded = true;
    }
}

MAKE_HOOK_MATCH(SaberModelController_init, &GlobalNamespace::SaberModelController::Init, void,  GlobalNamespace::SaberModelController* self, UnityEngine::Transform* parent, GlobalNamespace::Saber* saber) {
    SaberModelController_init(self, parent, saber);

    if(!hasLoaded) return;

    logger.info("SaberModelController_init");
    GaySabers::SaberColorManager::StartColorCoroutine(self, saber);
}

extern "C" __attribute__((visibility("default"))) void setup(CModInfo& info) {
    info.id = modInfo.id.c_str();
    info.version = modInfo.version.c_str();
    info.version_long = modInfo.versionLong;

    Paper::Logger::RegisterFileContextId(MOD_ID);

    getConfig().Load();
    logger.info("Completed setup!");
}

extern "C" __attribute__((visibility("default"))) void load() {
    il2cpp_functions::Init();
    BSML::Init();
    BSML::Register::RegisterSettingsMenu("GaySabers", DidActivate, false);

    if(!getConfig().config.HasMember("Enabled")) {
        getConfig().config.AddMember("Enabled", true, getConfig().config.GetAllocator());
        getConfig().Write();
    }

    logger.info("Registered Mod Settings!");


    logger.info("Installing hooks...");
    INSTALL_HOOK(logger, SaberModelController_init);
    INSTALL_HOOK(logger, MainMenuViewControllerDidActivate);
    logger.info("Installed all hooks!");
}
