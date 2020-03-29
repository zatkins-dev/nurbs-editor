#include "Menu.h"

Menu::Menu(Screen* screen) {
    bool enabled = true;
    FormHelper* gui = new FormHelper(screen);
    nanoguiWindow = gui->add_window(Vector2i(10, 10), "Form helper example");
    gui->add_group("Basic types");
    gui->add_variable("bool", bvar)->set_tooltip("Test tooltip.");
    gui->add_variable("string", strval);

    gui->add_group("Validating fields");
    gui->add_variable("int", ivar)->set_spinnable(true);
    gui->add_variable("float", fvar)->set_tooltip("Test.");
    gui->add_variable("double", dvar)->set_spinnable(true);

    gui->add_group("Complex types");
    gui->add_variable("Enumeration", enumval, enabled)->set_items({"Item 1", "Item 2", "Item 3"});
    gui->add_variable("Color", colval)->set_final_callback([](const Color& c) {
        std::cout << "ColorPicker Final Callback: [" << c.r() << ", " << c.g() << ", " << c.b()
                  << ", " << c.w() << "]" << std::endl;
    });

    gui->add_group("Other widgets");
    gui->add_button("A button", []() { std::cout << "Button pressed." << std::endl; })
        ->set_tooltip(
            "Testing a much longer tooltip, that will wrap around to new lines multiple times.");
    ;

    screen->set_visible(true);
    screen->perform_layout();
    nanoguiWindow->center();
}