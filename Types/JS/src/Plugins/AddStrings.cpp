#include "js.hpp"

namespace GView::Type::JS::Plugins
{
using namespace GView::View::LexicalViewer;

static std::map<std::u16string, std::u16string> stringTokens{};

std::string_view AddStrings::GetName()
{
    return "Add Strings";
}
std::string_view AddStrings::GetDescription()
{
    return "Concatenate multiple strings that are beeing added.";
}
bool AddStrings::CanBeAppliedOn(const GView::View::LexicalViewer::PluginData& data)
{
    // this handles declaration & assignment only => var x = "string";
    // this doesn't handle assignment => var x = "string"; x = "other_string";
    stringTokens.clear();

    // mapping var(str)
    for (auto index = data.startIndex; index < data.endIndex; index++)
    {
        if (data.endIndex - index < 4)
        {
            break;
        }

        const auto& tFirst  = data.tokens[index];
        const auto& tMiddle = data.tokens[index + 1];
        const auto& tThird  = data.tokens[index + 2];

        const auto& tFourth = data.tokens[index + 3];
        const auto& tFifth  = data.tokens[index + 4];

        const auto& tFirstType  = tFirst.GetTypeID(TokenType::None);
        const auto& tMiddleType = tMiddle.GetTypeID(TokenType::None);
        const auto& tThirdType  = tThird.GetTypeID(TokenType::None);
        const auto& tFourthType = tFourth.GetTypeID(TokenType::None);
        const auto& tFifthType  = tFifth.GetTypeID(TokenType::None);

        const auto& tFirstText  = tFirst.GetText();
        const auto& tSecondText = tMiddle.GetText();
        const auto& tThirdText  = tThird.GetText();
        const auto& tFourthText = tFourth.GetText();
        const auto& tFifthText  = tFifth.GetText();

        if (tFirstType == TokenType::DataType_Var && tMiddleType == TokenType::Word && tThirdType == TokenType::Operator_Assignment &&
            tFourthType == TokenType::String && tFifthType == TokenType::Semicolumn)
        {
            const auto tSecondextU16  = std::u16string{ tSecondText };
            const auto tFourthTextU16 = std::u16string{ tFourthText };

            if (stringTokens.count(tSecondextU16) == 0)
            {
                stringTokens[tSecondextU16] = tFourthTextU16;
            }
        }
    }

    // actual condition check
    for (auto index = data.startIndex; index < data.endIndex; index++)
    {
        if (data.endIndex - index < 3)
        {
            break;
        }

        const auto& tFirst  = data.tokens[index];
        const auto& tMiddle = data.tokens[index + 1];
        const auto& tThird  = data.tokens[index + 2];

        const auto& tFirstType  = tFirst.GetTypeID(TokenType::None);
        const auto& tMiddleType = tMiddle.GetTypeID(TokenType::None);
        const auto& tThirdType  = tThird.GetTypeID(TokenType::None);

        const auto& tFirstText  = tFirst.GetText();
        const auto& tSecondText = tMiddle.GetText();
        const auto& tThirdText  = tThird.GetText();

        if (tFirstType == TokenType::String && tMiddleType == TokenType::Operator_Plus && tThirdType == TokenType::String)
        {
            return true;
        }

        if (tFirstType == TokenType::Word && tMiddleType == TokenType::Operator_Plus && tThirdType == TokenType::Word)
        {
            const auto tFirstText = std::u16string{ tFirst.GetText() };
            const auto tThirdText = std::u16string{ tThird.GetText() };

            if (stringTokens.count(tFirstText) && stringTokens.count(tThirdText))
            {
                return true;
            }
        }

        if (tFirstType == TokenType::String && tMiddleType == TokenType::Operator_Plus && tThirdType == TokenType::Word)
        {
            const auto tThirdText = std::u16string{ tThird.GetText() };

            if (stringTokens.count(tThirdText))
            {
                return true;
            }
        }

        if (tFirstType == TokenType::Word && tMiddleType == TokenType::Operator_Plus && tThirdType == TokenType::String)
        {
            const auto tFirstText = std::u16string{ tFirst.GetText() };

            if (stringTokens.count(tFirstText))
            {
                return true;
            }
        }
    }

    return false;
}
GView::View::LexicalViewer::PluginAfterActionRequest AddStrings::Execute(GView::View::LexicalViewer::PluginData& data)
{
    auto index = (int32) data.endIndex - 1;
    LocalUnicodeStringBuilder<256> temp;
    while (index >= (int32) data.startIndex)
    {
        const auto& tFirst  = data.tokens[index];
        const auto& tMiddle = tFirst.Precedent();
        const auto& tThird  = tFirst.Precedent().Precedent();

        const auto& tFirstType  = tFirst.GetTypeID(TokenType::None);
        const auto& tMiddleType = tMiddle.GetTypeID(TokenType::None);
        const auto& tThirdType  = tThird.GetTypeID(TokenType::None);

        const auto& tFirstText  = tFirst.GetText();
        const auto& tSecondText = tMiddle.GetText();
        const auto& tThirdText  = tThird.GetText();

        /*
            str + str
            var(str) + var(str)
            str + var(str)
            var(str) + str
        */

        if ((tFirstType == TokenType::String && tMiddleType == TokenType::Operator_Plus && tThirdType == TokenType::String) ||
            (tFirstType == TokenType::Word && stringTokens.count(std::u16string{ tFirst.GetText() }) &&
             tMiddleType == TokenType::Operator_Plus && tThirdType == TokenType::Word &&
             stringTokens.count(std::u16string{ tThird.GetText() })) ||
            (tFirstType == TokenType::String && tMiddleType == TokenType::Operator_Plus && tThirdType == TokenType::Word) &&
                  stringTokens.count(std::u16string{ tThird.GetText() }) ||
            (tFirstType == TokenType::Word && stringTokens.count(std::u16string{ tFirst.GetText() }) &&
             tMiddleType == TokenType::Operator_Plus && tThirdType == TokenType::String))
        {
            Token start = tThird;

            while (start.Precedent().GetTypeID(TokenType::None) == TokenType::Operator_Plus &&
                         start.Precedent().Precedent().GetTypeID(TokenType::None) == TokenType::String ||
                   (start.Precedent().Precedent().GetTypeID(TokenType::None) == TokenType::Word &&
                    stringTokens.count(std::u16string{ start.Precedent().Precedent().GetText() }) > 0))
            {
                start = start.Precedent().Precedent();
            }

            temp.Clear();
            temp.AddChar('"');

            index            = start.GetIndex();
            auto startOffset = start.GetTokenStartOffset();
            auto endOffset   = tFirst.GetTokenEndOffset();
            if (!startOffset.has_value() || !endOffset.has_value())
                return GView::View::LexicalViewer::PluginAfterActionRequest::None;

            auto size = endOffset.value() - startOffset.value();
            while (start.GetIndex() <= tFirst.GetIndex())
            {
                const auto& txt = start.GetTypeID(TokenType::None) == TokenType::Word // get var(str) or '+'
                                        ? stringTokens.at(std::u16string{ start.GetText() })
                                        : start.GetText();
                auto value      = txt.substr(1, txt.length() - 2);
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
                // temp.Add(txt.substr(1, txt.length() - 2));
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