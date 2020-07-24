#ifndef TURTLE_HPP
#define TURTLE_HPP
/*
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <iostream>
#include <fstream>
#include <streambuf>
#define NEWLINE "\n"
namespace turtle {

    class rdf;
    inline void write_to_file(std::string path, rdf graph);
    inline rdf read_from_file(std::string path);

    struct triple {
        std::string subject;
        std::string predicat;
        std::string object;
    };

    struct rdf {
        /// friend void write_to_file(std::string path, rdf_graph graph);
        ///   friend rdf_graph read_from_file(std::string path);
        std::map<std::string, std::string> prefixes;
        std::vector<triple> triples;

        inline void addPrefix(std::string prefixShortcut, std::string prefixURI) {
            prefixes[prefixShortcut] = prefixURI;
        }
    };

    inline rdf beautify(rdf r) {
        auto replaceStr = [](std::string& strToMod, const std::string& val, const std::string& replacement) {
            while (strToMod.find(val) != std::string::npos)
                strToMod.replace(strToMod.find(val), val.size(), replacement);
        };
        for (auto triple : r.triples) {
            for (auto prefix : r.prefixes) {
                replaceStr(triple.subject, prefix.first, prefix.second);
                replaceStr(triple.predicat, prefix.first+":" , prefix.second);
                //replaceStr(triple.object, prefix.first+":", prefix.second);
            }
        }
        return r;
    }

    class TurtleException :public std::exception {
        std::string whatMsg;
    public:
        inline TurtleException(std::string what) {
            this->whatMsg = what;
        }
        inline virtual char const* what() const override {
            return whatMsg.c_str();
        }
    };

    inline std::string get_as_ttl_string(rdf graph) {
        std::string ttl_file_content;
        for (auto prefix : graph.prefixes) {
            ttl_file_content += "@prefix " + prefix.first + ": <" + prefix.second + "> ."+ NEWLINE;
        }
        std::string lastSubject = "";
        std::string lastPredicate = "";
        for (int i = 0; i < graph.triples.size(); i++) {
       
            std::string next_seperator = std::string(" ") + (i + 1 == graph.triples.size() ? "." :
                graph.triples[i].subject == graph.triples[i + 1].subject ?
                ";" : graph.triples[i].predicat == graph.triples[i + 1].predicat ? "," : ".");
            std::string subject = graph.triples[i].subject==lastSubject?"\t": graph.triples[i].subject;
            std::string predicate = graph.triples[i].predicat == lastPredicate ? "\t" : graph.triples[i].predicat;

           
            ttl_file_content += graph.triples[i].subject +" "+ graph.triples[i].predicat +" "+ graph.triples[i].object + " ."+ NEWLINE;
            lastPredicate = graph.triples[i].predicat;
            lastSubject = graph.triples[i].subject;
        }
        return ttl_file_content;
    }

    enum class TokenType {
        IRIREF,
        PNAME_NS,
        PNAME_LN,
        BLANK_NODE_LABEL,
        LANGTAG,
        INTEGER,
        DECIMAL,
        DOUBLE,
        EXPONENT,
        STRING_LITERAL_QUOTE,
        STRING_LITERAL_SINGLE_QUOTE,
        STRING_LITERAL_LONG_SINGLE_QUOTE,
        STRING_LITERAL_LONG_QUOTE,
        UCHAR,
        ECHAR,
        WS,
        ANON,
        PN_CHARS_BASE,
        PN_CHARS_U,
        PN_CHARS,
        PN_PREFIX,
        PN_LOCAL,
        PLX,
        PERCENT,
        HEX,
        PN_LOCAL_ESC
    };
    struct TurtleToken {
        TokenType tokentype;
        std::string text;
    };
#define TERMINAL(Name, regex) {#Name,regex}
     class TurtleLexer {
     protected:
     
         std::string input;
         int p ;// inedex of current input
         char c;
         std::vector<std::tuple<std::string, std::string>> terminals = {
         TERMINAL(PERCENT,"%HEXHEX"),
         TERMINAL(HEX,"([0-9]|[A-F]|[a-f])"),
         TERMINAL(PN_LOCAL_ESC,"\\\\(_|~|.|-|!|$|&|'|(|)|\\*|\\+|,|;|=|\\/|\\?|#|@|%)")
         };
     public:
         TurtleLexer(std::string content) {
             p = 0;
             this->input = content;
             c = input[p];
             for (auto& x : terminals) {
                 for (auto& y : terminals) {
                     //Wenn terminal x ein wort enthält, welches identisch zu irgendeinem key aus terminals y ist, tausche es aus.
                     while (std::get<1>(x).find(std::get<0>(y)) != std::string::npos)
                         std::get<1>(x).replace(std::get<1>(x).find(std::get<0>(y)), std::get<0>(y).size(), std::get<1>(y));
                 }
             }
         }


         TurtleToken nextToken() {
             while (c != '\0') {
                 switch (c) {
                     case '\\':
                         return PN_LOCAL_ESC();
                 }




                 // 2  size Lexingrules
          
             }
         }

         TurtleToken PN_LOCAL_ESC() {
             regToken(std::regex("\\\\(_|~|.|-|!|$|&|'|(|)|\\*|\\+|,|;|=|\\/|\\?|#|@|%)"),2,TokenType::PN_LOCAL_ESC);
         }
         void consume() {
             p++;
             if (p >= input.size())c = '\0';
             else c = input[p];
         }

         void match(char x) {
             if (c == x)consume();
             else throw TurtleException(std::string("expecting ") + x + "; found " + c);
         }

         TurtleToken regToken(std::regex regEx, size_t length, TokenType t) {
             TurtleToken tk{t, "" };
             std::smatch sm;
             if (std::regex_match(this->input.cbegin() + p, this->input.cbegin()+p+length, sm, regEx)) {
                 tk.text = sm[0].str();
             }
             for (int i = 0; i < tk.text.size(); i++)
                 consume();
             return tk;
         }

     };
 /*    class TurtleParser {
     protected:
         std::string content;
         char c;
     public:
         TurtleToken nextToken() {
             while (c != '\0') {

             }
         }
     };*/
    
   /*
    inline std::vector<TurtleToken> lexing(std::string filecontent) {
        std::vector<TurtleToken> tokens;


        
     /*   for (auto & x : terminals) {
            for (auto & y : terminals) {
                //Wenn terminal x ein wort enthält, welches identisch zu irgendeinem key aus terminals y ist, tausche es aus.
                while (std::get<1>(x).find(std::get<0>(y)) != std::string::npos) 
                    std::get<1>(x).replace(std::get<1>(x).find(std::get<0>(y)), std::get<0>(y).size(), std::get<1>(y));
            }
        }
        for (int i = 0; i < filecontent.size(); i=i) {
            TurtleToken t;
            for (auto x : terminals) {
                std::smatch sm;
                //resolve regex;
             
                std::regex reg{ std::get<1>(x)};

                if (std::regex_match(filecontent.cbegin() + i, filecontent.cend(), sm, reg, 
                    match_not_bol|match_not_eol| match_continuous| format_first_only)) {
                    auto curT = TurtleToken{ std::get<0>(x) , sm[0].str() };
                    if (curT.text.size() >= t.text.size())
                        t = curT;
                }
            }
            if (t.text.empty()) {
                t= TurtleToken{ "UNKNOWN" , filecontent.substr(i,1) };
            }

            tokens.push_back(t);
            i += t.text.size();
        }*//*
        return tokens;
    }

    inline rdf read_from_file(std::string path) {

        std::ifstream t(path);
        std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        // lexing(str);
        for (auto tok : lexing("%99_")) {
        //    std::cout << "<" << tok.tokentype << ">" << " " << tok.text << std::endl;
        }

        rdf x;
        return x;
    }
}*/
#endif //!  TURTLE_HPP
