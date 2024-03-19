// VT-Rex
// Copyright (c) 2024 James Holderness
// Distributed under the MIT License

#include "font.h"

#include "capabilities.h"

#include <iostream>

constexpr auto font_10x16 = R"(0;1;1;10;0;2;16;0{ @
F^~}}~~~~~/???BN~~^N^/?????NK???;
~?????????/BB????????/??????????;
??????????/_____OWCCK/??????????;
??????????/O_________/?????CC???;
??????????/_????_____/@AAAA@????;
{~BZB~~~~{/~~~~^RRRBB/NNNNKK????;
????B~NFBF/?????BB???/??????????;
????????_o/??oGGGK?B?/B?@?@AAAAA;
??????????/C[o???????/AAAB??????;
??????????/_OGCCOO___/???????G??;
??????????/___OGCCOO_/??????????;
??????????/o{[{{{{{{o/NNMNNNNNNN;
??????????/__________/??????????;
??????????/??????????/??????????;
F^~}}~~~~~/___BN~~^N^/?????NK???;
??oGGGwo??/?N~o???~N?/???@@@@???;
???oww????/????~~????/??@@@@@@??;
?owGGGGwo?/?o{MEEBB@?/?@@@@@@@@?;
??GGGgwWW?/?ooaBBe}{?/??@@@@@@??;
????_oww??/?KMJHG~~G?/??????@@??;
?wwGGGGG??/?rrAAAA}{?/??@@@@@@??;
?_oWGGGG??/?~~AAAA}{?/??@@@@@@??;
?wwGGGGww?/????{}B@??/????@@????;
?owGGGWo??/?xfEEEKzw?/??@@@@@@??;
?owGGGGwo?/?@BAAaq^N?/??@@@@????;
??????????/_????????_/NK????KMNN;
w??????_ow/^~{ww{~~~~/?@BNNNNNNN;
{~f~~~~~~{/~~~~^RRRBB/NNNNKK????;
??????????/__________/C??GG????C;
??????????/??????????/??????????;
GGKCKW_???/??????@BAA/AAAAAAAAAA;
???_oGGKCK/GK?B??????/AAAAAAAAAA;
??oWo?????/?~CCC~????/??????????;
??????????/??????????/??????????;
??????????/??????????/??????????;
??????????/??????????/??????????;
???wGGGG??/???~aaa_??/??????????;
??????????/??????????/??????????;
?oWGG?????/FWoa}?????/??????????;
?ww????ww?/?~~AAAA~~?/?@@????@@?;
??GGwwGG??/????~~????/??@@@@@@??;
??????????/??????????/??????????;
~~~~FCCC??/~~~^BN????/NNB???????;
??????????/??????????/??????????;
??ww_ww???/??~?B?~???/??????????;
??????????/??????????/??????????;
?_WGW_????/?No_oN????/??????????;
??????????/??????????/??????????;
??????????/??????????/??????????;
????wGGGo?/????~CMYr?/??????????;
???}~Fff@B/???~~?^^]^/???FNMMMMM;
f~ffF~}???/^^^^?~~???/MMMMMNF???;
??????????/??????????/??????????;
??w???w???/??BMwMB???/??????????;
???_o_????/???~~~?oo?/NN?NNN?NN?;
????_o_???/?ow?~~~?oo/?NNGNNN?NN;
?????__???/??{{?~~?__/??NN?NN?NN;
?????_o???/??{{?~~?{{/??NN?NN?NN;
??????????/??????????/??????????;
~~~F?B????/~B?_______/NK????????;
??????????/??????????/??????????;
F^~}}~~~~~/___BN^~nN^/??????@@??;
??????????/__________/????CC????;
~~~F?B????/~r??______/??????????;
??????????/????_oo_??/????NNNN??;
??????????/????????ow/????GKG?NN;
??????????/wo????????/NN????????;
??????????/??????????/??????MNM?;
??????????/??????????/???????GG?;
??????????/?_oo_?????/?NNNN?????;
_o_?~~~~?w/~~~?~~~~?~/NNNGNNNNMN;
{w????????/~~????????/NF????????;
{w??~~~?~~/~~??~~~_~~/NF??@FFFNN;
~~?owo????/~~?~~~????/NN?NNN????;
{w????~~~?/~~?}{?~~~[/NF?NN?NNN?;
}}?????~~?/NF?oo??~~o/KK?NN?G@BB;
?~~~~?owo?/o~~~~?~~~?/BNNNN?NNF?;
@BFF~~~~@@/___?~~~~?_/??GGNNNN??;
????????~~/_______?~~/???????GNN;
~~FFB?????/~~?_______/NN????????;
???F^[~~~?/_____?~~~?/??????NNN?;
~~?~~_~^??/_`B~~@____/???NN?????;
?~~~~BB@??/?~~~~?____/GNNNN?????;
??????????/??????????/??????????;
??????????/??????????/??????????;
??????????/??????????/??????????;
^~_~~~KNB?/_`@~~~?___/???NNN????;
?@FE~~~o~N/___?~~~?__/????NNN???;
??^~o~~?~~/____B~~Nfb/?????NN???;
??BNK~~o~N/____?~~?__/?????NN???;
??????????/???????oGG/?????B?@?@;
~~~F?B????/~B????????/NK????????;
W_????????/?@BAAC[o??/AAAAAAAAB?;
??????????/__________/??G????A??
)";

soft_font::soft_font(const capabilities& caps)
    : _has_soft_fonts{caps.has_soft_fonts}
{
    if (_has_soft_fonts) {
        auto font_data = std::string{font_10x16};
        // Some terminals (like RLogin) will not cope with DECDLD content
        // containing newlines, so we need to strip those out first.
        for (auto i = 0; i < font_data.size(); i++)
            if (font_data[i] == '\n')
                font_data.erase(i--, 1);
        std::cout << "\033P" << font_data << "\033\\";
        std::cout << "\033( @";
    }
}

soft_font::~soft_font()
{
    // Make sure the ASCII character set is restored on exit.
    std::cout << "\033(B";
    // And if we've created a font, erase the font buffers on exit.
    if (_has_soft_fonts)
        std::cout << "\033P0;0;2{ @\033\\";
}
