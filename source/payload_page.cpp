#include "payload_page.hpp"
 
namespace i18n = brls::i18n;
using namespace i18n::literals;
PayloadPage::PayloadPage() : AppletFrame(true, true)
{
    CFW cfw = getCFW();
    this->setTitle("menus/payload_reboot"_i18n );
    list = new brls::List();
    label = new brls::Label(
        brls::LabelStyle::DESCRIPTION,
        "menus/payload_select"_i18n ,
        true
    );
    list->addView(label);
    std::vector<std::string> payloads = fetchPayloads();
    int nbPayloads = payloads.size();
    items.reserve(nbPayloads);
    for (int i = 0; i < nbPayloads; i++){
        std::string payload = payloads[i];
        items[i] = new brls::ListItem(payload);
        items[i]->getClickEvent()->subscribe([&, payload](brls::View* view) {
            reboot_to_payload(payload.c_str());
            brls::Application::popView();
        });
        if(cfw == ams){
            items[i]->registerAction("menus/payload_set"_i18n , brls::Key::X, [this, payload] { 
                std::string res1;
                if(R_SUCCEEDED(CopyFile(payload.c_str(), REBOOT_PAYLOAD_PATH))){
                    res1 += "menus/payload_success"_i18n  + payload + "menus/payload_to"_i18n  + std::string(REBOOT_PAYLOAD_PATH) + "'.";
                    
                }
                else{
                    res1 += "Failed.";
                }
                brls::Dialog* dialog = new brls::Dialog(res1);
                brls::GenericEvent::Callback callback = [dialog](brls::View* view) {
                    dialog->close();
                };
                dialog->addButton("menus/payload_ok"_i18n , callback);
                dialog->setCancelable(true);
                dialog->open();
                return true;
            });
        }
        items[i]->registerAction("menus/payload_set_up"_i18n , brls::Key::Y, [this, payload] { 
                std::string res2;
                if(R_SUCCEEDED(CopyFile(payload.c_str(), UPDATE_BIN_PATH))){
                    res2 += "menus/payload_success"_i18n  + payload + "menus/payload_to"_i18n  + std::string(UPDATE_BIN_PATH) + "'.";
                }
                else{
                    res2 += "Failed.";
                }
                brls::Dialog* dialog = new brls::Dialog(res2);
                brls::GenericEvent::Callback callback = [dialog](brls::View* view) {
                    dialog->close();
                };
                dialog->addButton("menus/payload_ok"_i18n , callback);
                dialog->setCancelable(true);
                dialog->open();
                return true;
            });
        list->addView(items[i]);
    }
    list->addView(new brls::ListItemGroupSpacing(true));

    shutDown = new brls::ListItem("menus/payload_shut"_i18n );
    shutDown->getClickEvent()->subscribe([](brls::View* view) {
        shut_down(false);
        brls::Application::popView();
    });
    list->addView(shutDown);

    reboot = new brls::ListItem("menus/payload_reboot_2"_i18n );
    reboot->getClickEvent()->subscribe([](brls::View* view) {
        shut_down(true);
        brls::Application::popView();
    });
    list->addView(reboot);

    this->setContentView(list);

}