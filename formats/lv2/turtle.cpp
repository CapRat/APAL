/*#include "turtle.hpp"
using namespace turtle;
int main(int argc, char* argv[]) {
    //std::string ttlPath(argv[1]);
   // turtle::read_from_file(ttlPath);
    rdf rd{
        {
            {"rdf","http://www.w3.org/1999/02/22-rdf-syntax-ns#"},
            {"rdfs","http://www.w3.org/2000/01/rdf-schema#"},
            {"lv2","http://lv2plug.in/ns/lv2core#"},
            {"owl","http://www.w3.org/2002/07/owl#"}
        },{
            {"<http://lv2plug.in/ns/lv2core>", "a", "owl:Ontology"},
            {"<http://lv2plug.in/ns/lv2core>", "rdfs:label", "LV2"},
            {"<http://lv2plug.in/ns/lv2core>", "rdfs:comment", "An extensible open standard for audio plugins."}
         }
    };

    std::cout << get_as_ttl_string(beautify(rd));
}*/