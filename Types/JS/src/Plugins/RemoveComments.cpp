#include "js.hpp"

namespace GView::Type::JS::Plugins
{
using namespace GView::View::LexicalViewer;

std::string_view RemoveComments::GetName()
{
    return "Remove Comments";
}
std::string_view RemoveComments::GetDescription()
{
    return "Remove comments in the file.";
}
bool RemoveComments::CanBeAppliedOn(const GView::View::LexicalViewer::PluginData& data)
{
    for (auto index = data.startIndex; index < data.endIndex; index++)
    {
        if (data.tokens[index].GetTypeID(TokenType::None) == TokenType::Comment)
        {
            return true;
        }
    }
    return false;
}
GView::View::LexicalViewer::PluginAfterActionRequest RemoveComments::Execute(GView::View::LexicalViewer::PluginData& data)
{
    auto len    = std::min<>(data.tokens.Len(), data.endIndex);
    int32 index = static_cast<int32>(len) - 1;

    while (index >= (int32) data.startIndex)
    {
        Token token = data.tokens[index];
        if (!token.IsValid())
            break;
        if (token.GetTypeID(TokenType::None) == TokenType::Comment)
        {
            auto start = token.GetTokenStartOffset();
            auto end   = token.GetTokenEndOffset();
            if (start.has_value() && end.has_value())
            {
                data.editor.Delete(start.value(), end.value() - start.value());
            }
        }
        index--;
    }
    return GView::View::LexicalViewer::PluginAfterActionRequest::Rescan;
}
} // namespace GView::Type::JS::Plugins