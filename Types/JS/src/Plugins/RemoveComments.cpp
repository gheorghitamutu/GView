#include "js.hpp"

namespace GView::Type::JS::Plugins
{
using namespace GView::View::LexicalViewer;

std::string_view RemoveComments::GetName()
{
    return "Remove all comments.";
}

std::string_view RemoveComments::GetDescription()
{
    return "Removes all comments from the file.";
}

bool RemoveComments::CanBeAppliedOn(const PluginData& data)
{
    const auto len = std::min<>(data.tokens.Len(), data.endIndex);
    for (auto index = data.startIndex; index < len; index++)
    {
        if (data.tokens[index].GetTypeID(TokenType::None) == TokenType::Comment)
        {
            return true;
        }
    }

    return false;
}

PluginAfterActionRequest RemoveComments::Execute(PluginData& data)
{
    const auto len = std::min<>(data.tokens.Len(), data.endIndex);
    int32 index    = static_cast<int32>(len) - 1;
    while (index >= static_cast<int32>(data.startIndex))
    {
        auto token = data.tokens[index];
        CHECKBK(token.IsValid(), "");

        if (token.GetTypeID(TokenType::None) == TokenType::Comment)
        {
            const auto start = token.GetTokenStartOffset();
            const auto end   = token.GetTokenEndOffset();
            if ((start.has_value()) && (end.has_value()))
            {
                data.editor.Replace(start.value(), end.value() - start.value(), " ");
            }
        }
        index--;
    }

    return PluginAfterActionRequest::Rescan;
}
} // namespace GView::Type::JS::Plugins