#include <scwx/qt/util/json.hpp>

#include <fstream>

#include <boost/log/trivial.hpp>

namespace scwx
{
namespace qt
{
namespace util
{
namespace json
{

static const std::string logPrefix_ = "[scwx::qt::util::json] ";

/* Adapted from:
 * https://www.boost.org/doc/libs/1_77_0/libs/json/doc/html/json/examples.html#json.examples.pretty
 *
 * Copyright (c) 2019, 2020 Vinnie Falco
 * Copyright (c) 2020 Krystian Stasiowski
 * Distributed under the Boost Software License, Version 1.0. (See
 * http://www.boost.org/LICENSE_1_0.txt)
 */
static void PrettyPrintJson(std::ostream&             os,
                            boost::json::value const& jv,
                            std::string*              indent = nullptr);

bool FromJsonString(const boost::json::object& json,
                    const std::string&         key,
                    std::string&               value,
                    const std::string&         defaultValue)
{
   const boost::json::value* jv    = json.if_contains(key);
   bool                      found = false;

   if (jv != nullptr)
   {
      if (jv->is_string())
      {
         value = boost::json::value_to<std::string>(*jv);
         found = true;
      }
      else
      {
         BOOST_LOG_TRIVIAL(warning)
            << logPrefix_ << key
            << " is not a string, setting to default: " << defaultValue;
         value = defaultValue;
      }
   }
   else
   {
      BOOST_LOG_TRIVIAL(debug)
         << logPrefix_ << key
         << " is not present, setting to default: " << defaultValue;
      value = defaultValue;
   }

   return found;
}

boost::json::value ReadJsonFile(const std::string& path)
{
   std::ifstream ifs {path};
   std::string   line;

   boost::json::stream_parser p;
   boost::json::error_code    ec;

   while (std::getline(ifs, line))
   {
      p.write(line, ec);
      if (ec)
      {
         BOOST_LOG_TRIVIAL(warning) << logPrefix_ << ec.message();
         return nullptr;
      }
   }

   p.finish(ec);
   if (ec)
   {
      BOOST_LOG_TRIVIAL(warning) << logPrefix_ << ec.message();
      return nullptr;
   }

   return p.release();
}

void WriteJsonFile(const std::string&        path,
                   const boost::json::value& json,
                   bool                      prettyPrint)
{
   std::ofstream ofs {path};

   if (prettyPrint)
   {
      PrettyPrintJson(ofs, json);
   }
   else
   {
      ofs << json;
   }
   ofs.close();
}

static void PrettyPrintJson(std::ostream&             os,
                            boost::json::value const& jv,
                            std::string*              indent)
{
   std::string indent_;
   if (!indent)
      indent = &indent_;
   switch (jv.kind())
   {
   case boost::json::kind::object:
   {
      os << "{\n";
      indent->append(4, ' ');
      auto const& obj = jv.get_object();
      if (!obj.empty())
      {
         auto it = obj.begin();
         for (;;)
         {
            os << *indent << boost::json::serialize(it->key()) << " : ";
            PrettyPrintJson(os, it->value(), indent);
            if (++it == obj.end())
               break;
            os << ",\n";
         }
      }
      os << "\n";
      indent->resize(indent->size() - 4);
      os << *indent << "}";
      break;
   }

   case boost::json::kind::array:
   {
      os << "[\n";
      indent->append(4, ' ');
      auto const& arr = jv.get_array();
      if (!arr.empty())
      {
         auto it = arr.begin();
         for (;;)
         {
            os << *indent;
            PrettyPrintJson(os, *it, indent);
            if (++it == arr.end())
               break;
            os << ",\n";
         }
      }
      os << "\n";
      indent->resize(indent->size() - 4);
      os << *indent << "]";
      break;
   }

   case boost::json::kind::string:
   {
      os << boost::json::serialize(jv.get_string());
      break;
   }

   case boost::json::kind::uint64: os << jv.get_uint64(); break;

   case boost::json::kind::int64: os << jv.get_int64(); break;

   case boost::json::kind::double_: os << jv.get_double(); break;

   case boost::json::kind::bool_:
      if (jv.get_bool())
         os << "true";
      else
         os << "false";
      break;

   case boost::json::kind::null: os << "null"; break;
   }

   if (indent->empty())
      os << "\n";
}

} // namespace json
} // namespace util
} // namespace qt
} // namespace scwx
