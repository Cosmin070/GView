#include "js.hpp"

namespace GView::Type::JS::Plugins
{
using namespace GView::View::LexicalViewer;

std::string_view MergeStrings::GetName()
{
    return "Merge strings";
}
std::string_view MergeStrings::GetDescription()
{
    return "Merge strings";
}
bool MergeStrings::CanBeAppliedOn(const GView::View::LexicalViewer::PluginData& data)
{
    for (auto index = data.startIndex; index < data.endIndex; index++)
    {
        if ((data.tokens[index].GetTypeID(TokenType::None) == TokenType::String ||
             data.tokens[index].GetTypeID(TokenType::None) == TokenType::Word) &&
            data.tokens[index + 1].GetTypeID(TokenType::None) == TokenType::Operator_Plus &&
            (data.tokens[index + 2].GetTypeID(TokenType::None) == TokenType::String ||
             data.tokens[index + 2].GetTypeID(TokenType::None) == TokenType::Word))
        {
            return true;
        }
    }
    return false;
}
GView::View::LexicalViewer::PluginAfterActionRequest MergeStrings::Execute(GView::View::LexicalViewer::PluginData& data)
{
    int32 index = static_cast<int32>(data.endIndex) - 1;
    LocalUnicodeStringBuilder<256> temp;
    while (index >= (int32) data.startIndex)
    {
        Token endToken = data.tokens[index];
        if ((endToken.GetTypeID(TokenType::None) == TokenType::String || endToken.GetTypeID(TokenType::None) == TokenType::Word) &&
            endToken.Precedent().GetTypeID(TokenType::None) == TokenType::Operator_Plus &&
            (endToken.Precedent().Precedent().GetTypeID(TokenType::None) == TokenType::String ||
             endToken.Precedent().Precedent().GetTypeID(TokenType::None) == TokenType::Word))
        {
            Token start = endToken.Precedent().Precedent();
            while (start.Precedent().GetTypeID(TokenType::None) == TokenType::Operator_Plus &&
                   (start.Precedent().Precedent().GetTypeID(TokenType::None) == TokenType::String ||
                    start.Precedent().Precedent().GetTypeID(TokenType::None) == TokenType::Word))
            {
                start = start.Precedent().Precedent();
            }
            temp.Clear();
            temp.AddChar('"');
            index            = start.GetIndex();
            auto startOffset = start.GetTokenStartOffset();
            auto endOffset   = endToken.GetTokenEndOffset();
            if (!startOffset.has_value() || !endOffset.has_value())
                return GView::View::LexicalViewer::PluginAfterActionRequest::None;
            auto size = endOffset.value() - startOffset.value();
            while (start.GetIndex() <= endToken.GetIndex())
            {
                if (start.GetTypeID(TokenType::None) == TokenType::String)
                {
                    auto txt   = start.GetText();
                    auto value = txt.substr(1, txt.length() - 2);
                    if (value.find_first_of('"') == std::u16string_view::npos)
                    {
                        temp.Add(value);
                    }
                    else
                    {
                        for (auto ch : value)
                        {
                            if (ch == '"')
                                temp.AddChar('\\');
                            temp.AddChar(ch);
                        }
                    }
                }
                else
                {
                    auto txt              = start.GetText();
                    int32 indexOfVariable = (int32) data.startIndex + 1;
                    while (indexOfVariable < index)
                    {
                        Token temporary = data.tokens[indexOfVariable];
                        if (temporary.GetTypeID(TokenType::None) == TokenType::Word && temporary.GetText() == txt)
                        {
                            while (data.tokens[indexOfVariable + 1].GetTypeID(TokenType::None) != TokenType::Semicolumn)
                            {
                                auto variableValue = data.tokens[indexOfVariable + 2].GetText();
                                auto variableText  = variableValue.substr(1, variableValue.length() - 2);
                                if (variableText.find_first_of('"') == std::u16string_view::npos)
                                {
                                    temp.Add(variableText);
                                }
                                else
                                {
                                    for (auto ch : variableText)
                                    {
                                        if (ch == '"')
                                            temp.AddChar('\\');
                                        temp.AddChar(ch);
                                    }
                                }
                                indexOfVariable += 2;
                            }
                        }
                        indexOfVariable++;
                    }
                }
                start = start.Next().Next();
            }
            temp.AddChar('"');
            data.editor.Replace(startOffset.value(), size, temp.ToStringView());
        }
        index--;
    }
    return GView::View::LexicalViewer::PluginAfterActionRequest::Rescan;
}
} // namespace GView::Type::JS::Plugins