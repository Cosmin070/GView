#include "GViewApp.hpp"

using namespace GView::View;

GView::View::Factory::Factory(std::unique_ptr<GView::Object> obj)
{
    verticalPanels.reserve(32);
    horizontalPanels.reserve(16);
    views.reserve(8);
    fileObject = std::move(obj);
}
bool GView::View::Factory::AddPanel(std::unique_ptr<AppCUI::Controls::TabPage> ctrl, bool vertical)
{
    CHECK(ctrl, false, "Expecting a valid control !");
    if (vertical)
        verticalPanels.push_back(std::move(ctrl));
    else
        horizontalPanels.push_back(std::move(ctrl));
    return true;
}
Reference<Buffer::FactoryInterface> GView::View::Factory::CreateBufferView(const std::string_view& name)
{
    auto view = std::make_unique<Buffer::Factory>(name);
    auto ref = Reference<Buffer::FactoryInterface>(view.get());
    views.push_back(std::move(view));
    return ref;
}
