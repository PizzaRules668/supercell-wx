#include <scwx/wsr88d/rpg/text_and_special_symbol_packet.hpp>

#include <istream>
#include <string>

#include <boost/log/trivial.hpp>

namespace scwx
{
namespace wsr88d
{
namespace rpg
{

static const std::string logPrefix_ =
   "[scwx::wsr88d::rpg::text_and_special_symbol_packet] ";

class TextAndSpecialSymbolPacketImpl
{
public:
   explicit TextAndSpecialSymbolPacketImpl() :
       packetCode_ {0},
       lengthOfBlock_ {0},
       valueOfText_ {0},
       startI_ {0},
       startJ_ {0},
       characters_ {}
   {
   }
   ~TextAndSpecialSymbolPacketImpl() = default;

   uint16_t packetCode_;
   uint16_t lengthOfBlock_;
   uint16_t valueOfText_;
   int16_t  startI_;
   int16_t  startJ_;

   std::vector<char> characters_;
};

TextAndSpecialSymbolPacket::TextAndSpecialSymbolPacket() :
    p(std::make_unique<TextAndSpecialSymbolPacketImpl>())
{
}
TextAndSpecialSymbolPacket::~TextAndSpecialSymbolPacket() = default;

TextAndSpecialSymbolPacket::TextAndSpecialSymbolPacket(
   TextAndSpecialSymbolPacket&&) noexcept                       = default;
TextAndSpecialSymbolPacket& TextAndSpecialSymbolPacket::operator=(
   TextAndSpecialSymbolPacket&&) noexcept = default;

uint16_t TextAndSpecialSymbolPacket::packet_code() const
{
   return p->packetCode_;
}

uint16_t TextAndSpecialSymbolPacket::length_of_block() const
{
   return p->lengthOfBlock_;
}

std::optional<uint16_t> TextAndSpecialSymbolPacket::value_of_text() const
{
   std::optional<uint16_t> value;

   if (p->packetCode_ == 8)
   {
      value = p->valueOfText_;
   }

   return value;
}

size_t TextAndSpecialSymbolPacket::data_size() const
{
   return p->lengthOfBlock_ + 4u;
}

bool TextAndSpecialSymbolPacket::Parse(std::istream& is)
{
   bool blockValid = true;

   std::streampos isBegin = is.tellg();

   is.read(reinterpret_cast<char*>(&p->packetCode_), 2);
   is.read(reinterpret_cast<char*>(&p->lengthOfBlock_), 2);

   p->packetCode_    = ntohs(p->packetCode_);
   p->lengthOfBlock_ = ntohs(p->lengthOfBlock_);

   int textLength = static_cast<int>(p->lengthOfBlock_) - 4;

   is.read(reinterpret_cast<char*>(&p->startI_), 2);
   is.read(reinterpret_cast<char*>(&p->startJ_), 2);

   p->startI_ = ntohs(p->startI_);
   p->startJ_ = ntohs(p->startJ_);

   if (is.eof())
   {
      BOOST_LOG_TRIVIAL(debug) << logPrefix_ << "Reached end of file";
      blockValid = false;
   }
   else
   {
      if (p->packetCode_ != 1 && p->packetCode_ != 2 && p->packetCode_ != 8)
      {
         BOOST_LOG_TRIVIAL(warning)
            << logPrefix_ << "Invalid packet code: " << p->packetCode_;
         blockValid = false;
      }
      else if (p->lengthOfBlock_ < 1 || p->lengthOfBlock_ > 32767)
      {
         BOOST_LOG_TRIVIAL(warning)
            << logPrefix_ << "Invalid length of block: " << p->packetCode_;
         blockValid = false;
      }
      else if (p->packetCode_ == 8)
      {
         is.read(reinterpret_cast<char*>(&p->valueOfText_), 2);
         p->valueOfText_ = ntohs(p->valueOfText_);

         textLength -= 2;
      }
   }

   if (blockValid && textLength < 0)
   {
      BOOST_LOG_TRIVIAL(warning)
         << logPrefix_ << "Too few bytes in block: " << p->lengthOfBlock_;
      blockValid = false;
   }

   if (blockValid)
   {
      p->characters_.resize(textLength);
      is.read(reinterpret_cast<char*>(p->characters_.data()), textLength);
   }

   std::streampos isEnd = is.tellg();

   if (!ValidateMessage(is, isEnd - isBegin))
   {
      blockValid = false;
   }

   return blockValid;
}

std::shared_ptr<TextAndSpecialSymbolPacket>
TextAndSpecialSymbolPacket::Create(std::istream& is)
{
   std::shared_ptr<TextAndSpecialSymbolPacket> packet =
      std::make_shared<TextAndSpecialSymbolPacket>();

   if (!packet->Parse(is))
   {
      packet.reset();
   }

   return packet;
}

} // namespace rpg
} // namespace wsr88d
} // namespace scwx
