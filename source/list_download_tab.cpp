#include "list_download_tab.hpp"
 
namespace i18n = brls::i18n;
using namespace i18n::literals;

ListDownloadTab::ListDownloadTab(archiveType type) :
    brls::List()
{
    std::tuple<std::vector<std::string>, std::vector<std::string>> links;
    std::string operation = "menus/Getting"_i18n ;
    std::string firmwareText("menus/firmware_text"_i18n);

    std::string currentCheatsVer = 
                "menus/currentCeatsver"_i18n ;

    this->description = new brls::Label(brls::LabelStyle::DESCRIPTION, "", true);
    switch(type){
        case sigpatches:
            links = fetchLinks(SIGPATCHES_URL);
            operation += "menus/operation_1"_i18n ;
            this->description->setText(
                "menus/list_sigpatches"_i18n 
            );
            break;
        case fw:
            links = fetchLinks(FIRMWARE_URL);
            operation += "menus/operation_2"_i18n ;
            SetSysFirmwareVersion ver;
            if (R_SUCCEEDED(setsysGetFirmwareVersion(&ver))) firmwareText += ver.display_version;
            else firmwareText += "menus/list_not"_i18n ;
            this->description->setText(firmwareText);
            break;
        case app:
            std::get<0>(links).push_back("menus/list_latest"_i18n );
            std::get<1>(links).push_back(APP_URL);
            operation += "menus/list_app"_i18n ;
            break;
        case cfw:
            links = fetchLinks(CFW_URL);
            operation += "menus/list_cfw"_i18n ;
            this->description->setText(
                "menus/list_main"_i18n 
            );
            break;
        case cheats:
            std::string cheatsVer = fetchTitle(CHEATS_RELEASE_URL);
            if(cheatsVer != "-1"){
                std::get<0>(links).push_back("menus/list_latest_ver"_i18n  + cheatsVer + ")");
                switch(getCFW()){
                    case sxos:
                        std::get<1>(links).push_back(CHEATS_URL_TITLES);
                        break;
                    case ams:
                        std::get<1>(links).push_back(CHEATS_URL_CONTENTS);
                        break;
                    case rnx:
                        std::get<1>(links).push_back(CHEATS_URL_CONTENTS);
                        break;
                }
            }
            operation += "menus/list_cheats"_i18n ;
            currentCheatsVer += readVersion(CHEATS_VERSION);
            this->description->setText(currentCheatsVer);
            break;
    }

    this->addView(description);

    int nbLinks = std::get<0>(links).size();
    if(nbLinks){
        for (int i = 0; i<nbLinks; i++){
            std::string url = std::get<1>(links)[i];
            std::string text("menus/list_down"_i18n  + std::get<0>(links)[i] + "menus/list_from"_i18n  + url);
            listItem = new brls::ListItem(std::get<0>(links)[i]);
            listItem->setHeight(LISTITEM_HEIGHT);
            listItem->getClickEvent()->subscribe([&, text, url, type, operation](brls::View* view) {
                brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
                stagedFrame->setTitle(operation);
                stagedFrame->addStage(
                    new ConfirmPage(stagedFrame, text)
                );
                stagedFrame->addStage(
                    new WorkerPage(stagedFrame, "menus/list_downing"_i18n , [url, type](){downloadArchive(url, type);})
                );
                stagedFrame->addStage(
                    new WorkerPage(stagedFrame, "menus/list_extracting"_i18n , [type](){extractArchive(type);})
                );
                stagedFrame->addStage(
                    new ConfirmPage(stagedFrame, "menus/list_All"_i18n , true)
                );
                brls::Application::pushView(stagedFrame);
            });
            this->addView(listItem);
        }

    }

    else{
        notFound = new brls::Label(
            brls::LabelStyle::DESCRIPTION,
            "menus/list_could_done"_i18n ,
            true
        );
        notFound->setHorizontalAlign(NVG_ALIGN_CENTER);
        this->addView(notFound);
    }

    if(type == cheats){
        cheatSlipLabel = new brls::Label(
        brls::LabelStyle::DESCRIPTION,
        "menus/cheatslips_label"_i18n ,
            true
        );
        this->addView(cheatSlipLabel);
        cheatslipsItem = new brls::ListItem("menus/get_cheatslips"_i18n);
        cheatslipsItem->setHeight(LISTITEM_HEIGHT);
        cheatslipsItem->getClickEvent()->subscribe([&](brls::View* view) {
            if(std::filesystem::exists(TOKEN_PATH)) {
                brls::Application::pushView(new AppPage(true));
                return true;
            }
            else {
                SwkbdConfig kbd;
                char usr[0x100] = {0};
                char pwd[0x100] = {0};
                Result rc = swkbdCreate(&kbd, 0);
                if (R_SUCCEEDED(rc)) {
                    swkbdConfigMakePresetDefault(&kbd);
                    swkbdConfigSetOkButtonText(&kbd, "Submit");
                    swkbdConfigSetGuideText(&kbd, "www.cheatslips.com e-mail");
                    swkbdShow(&kbd, usr, sizeof(usr));
                    swkbdClose(&kbd);
                    rc = swkbdCreate(&kbd, 0);
                    if(R_SUCCEEDED(rc)){
                        swkbdConfigMakePresetPassword(&kbd);
                        swkbdConfigSetOkButtonText(&kbd, "Submit");
                        swkbdConfigSetGuideText(&kbd, "www.cheatslips.com password");
                        swkbdShow(&kbd, pwd, sizeof(pwd));
                        swkbdClose(&kbd);
                    }
                }
                std::string body =  "{\"email\":\"" + std::string(usr) 
                                    + "\",\"password\":\"" + std::string(pwd) + "\"}";
                nlohmann::json token = getRequest(CHEATSLIPS_TOKEN_URL, 
                    {"Accept: application/json", 
                    "Content-Type: application/json", 
                    "charset: utf-8"}, 
                body);
                if(token.find("token") != token.end()) {
                    std::ofstream tokenFile(TOKEN_PATH);
                    tokenFile << token.dump();
                    tokenFile.close();
                    brls::Application::pushView(new AppPage(true));
                    return true;
                }
                else {
                    brls::Dialog* dialog = new brls::Dialog("menus/wrong_cheatslips_id"_i18n);
                    brls::GenericEvent::Callback callback = [dialog](brls::View* view) {
                        dialog->close();
                    };
                    dialog->addButton("menus/Ok_button"_i18n , callback);
                    dialog->setCancelable(true);
                    dialog->open();
                    return true;
                }
            }
        });
        this->addView(cheatslipsItem);
    }

}

ListDownloadTab::~ListDownloadTab(){
    
}