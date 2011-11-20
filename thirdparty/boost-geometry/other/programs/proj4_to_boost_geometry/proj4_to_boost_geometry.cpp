// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2008, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Proj4 to Boost.Geometry


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <boost/tokenizer.hpp>


struct epsg_entry
{
    int epsg_code;
    std::string prj_name;
    std::string parameters;
    epsg_entry(int code, std::string const& name, std::string const& pars)
        : epsg_code(code)
        , prj_name(name)
        , parameters(pars)
    {}
};

std::vector<epsg_entry> epsg_entries;

#include "epsg_entries.inc"

const std::string tab1 = "    ";
const std::string tab2 = tab1 + tab1;
const std::string tab3 = tab2 + tab1;
const std::string tab4 = tab3 + tab1;
const std::string tab5 = tab4 + tab1;

const std::string include_projections = "#include <boost/geometry/extensions/gis/projections";

struct projection
{
    std::string direction; // inverse/forward/factors
    std::string model; // spheroid/ellipsoid/oblique/transverse/Guam-elliptical
    std::vector<std::string> lines;

    // Sort on model, then on direction
    inline bool operator<(projection const& other) const
    {
        return model == other.model ? direction < other.direction : model < other.model;
    }

};

struct define
{
    std::string name;
    std::string value;
};

struct derived
{
    std::string name;
    std::string description;
    std::string characteristics;
    std::vector<std::string> parsed_characteristics;
    std::vector<std::string> constructor_lines;
};

static bool find(std::string const& line, std::string const& tag, std::string::size_type& loc)
{
    loc = line.find(tag);
    return loc != std::string::npos;
}

inline void strip_comments(std::string& line)
{
    std::string::size_type loc = line.find("/*");
    if (loc == std::string::npos)
    {
        loc = line.find("//");
    }
    if (loc != std::string::npos)
    {
        line.erase(loc);
        boost::trim(line);
    }
}

inline void split(std::string const& line, std::vector<std::string>& subs, std::string const& seps1, std::string const& seps2 = "")
{
    typedef boost::tokenizer<boost::char_separator<char> > TOK;
    TOK tokens(line, boost::char_separator<char>(seps1.c_str(), seps2.c_str()));
    for (TOK::iterator it = tokens.begin(); it != tokens.end(); it++)
    {
        if (*it == seps2 && subs.size() > 0)
        {
            subs[subs.size() - 1] += *it;
        }
        else
        {
            subs.push_back(*it);
        }

    }
}



void replace_return(std::string& line, std::string const& var)
{
    std::string copy(line);
    boost::replace_all(copy, "return", "");
    boost::trim_left_if(copy, boost::is_any_of("\t ("));
    boost::trim_right_if(copy, boost::is_any_of("\t );"));
    if (copy == var)
    {
        line = tab3 + "return;";
    }
}


std::vector<std::string> extract_names(std::vector<std::string> const& parameters)
{
    std::string pars;
    for (std::vector<std::string>::const_iterator it = parameters.begin(); it != parameters.end(); it++)
    {
        std::string line = *it;
        strip_comments(line);
        std::string::size_type loc = line.find("[");
        if (loc != std::string::npos)
        {
            line.erase(loc);
            line += ";";
        }
        pars += line;
    }
    typedef boost::tokenizer<boost::char_separator<char> > TOK;
    std::vector<std::string> retval;
    TOK tokens(pars, boost::char_separator<char>(" ,;*"));
    for (TOK::iterator it = tokens.begin(); it != tokens.end(); it++)
    {
        if (! (*it == "double" || *it == "int" || *it == "void"))
        {
            retval.push_back(*it);
        }
    }
    return retval;
}

std::string end_entry(std::string const& line)
{
    // Process it
    std::string parameters = boost::trim_copy(line);
    boost::replace_all(parameters, "ENDENTRY", "");
    boost::replace_first(parameters, "(", "");
    boost::replace_last(parameters, ")", "");
    boost::replace_first(parameters, "(P", "(par");
    if (parameters == "P")
    {
        parameters.clear();
    }
    return parameters;
}


class proj4_converter
{
    public :
        proj4_converter(std::string const& filename, std::string const&  group, std::ostream& str)
            : stream(str)
            , projection_group(group)
            , hpp("BOOST_GEOMETRY_PROJECTIONS_" + boost::to_upper_copy(projection_group) + "_HPP")
            , skip_for_setup(false)
            , skip_lineno(0)
            , lineno(0)
        {
            parse(filename);
            write();
        }


    private :

        std::ostream& stream;


        void replace_pars(std::string& line, std::string const& prefix = "")
        {
            if (boost::contains(line, "->"))
            {
                boost::replace_all(line, "fac->", prefix + "fac.");
                boost::replace_all(line, "P->", prefix + "par.");
                boost::replace_all(line, "P -> ", prefix + "par.");
                // Refer to project-specific parameters
                for (std::vector<std::string>::const_iterator it = proj_parameter_names.begin();
                    it != proj_parameter_names.end(); it++)
                {
                    boost::replace_all(line, "par." + *it, "proj_parm." + *it);
                }
                // If the projection-parameter was a "p", it has replaced par.params/par.phi0 as well... Correct this
                boost::replace_all(line, "proj_parm.params", "par.params");
                boost::replace_all(line, "proj_parm.phi0", "par.phi0");
                // Same for lam0
                boost::replace_all(line, "proj_parm.lam0", "par.lam0");


                /*if (projection_group == "chamb")
                {
                    boost::replace_all(line, "par.c", "proj_parm.c");
                }
                if (use_mlfn())
                {
                    boost::replace_all(line, "par.en", "proj_parm.en");
                }*/
            }

            // Special casses
            if (projection_group == "robin"
                || projection_group == "tmerc")
            {
                if (boost::contains(line, "i = floor")
                    || boost::contains(line, "zone = floor"))
                {
                    boost::replace_all(line, "floor", "int_floor");
                }
            }
        }

        void replace_in_entry(std::string& line)
        {
            if (( projection_group == "sconics"  || projection_group == "lsat")
                && boost::contains(line, "seraz0"))
            {
                boost::replace_last(line, "P", "par, proj_parm");
            }
            else if ((projection_group == "sconics" && boost::contains(line, "phi12"))
                || (projection_group == "imw_p" && boost::contains(line, "phi12"))
                || (projection_group == "imw_p" && boost::contains(line, "xy")))
            {
                boost::replace_last(line, "(P", "(par, proj_parm");
            }

            replace_pars(line);

            if (boost::contains(line, "E_ERROR_0") && lineno == skip_lineno
                )
            {
                //std::cerr << lineno << " , " << skip_lineno << " : " << line << std::endl;
                line.clear();
            }

            boost::replace_all(line, "E_ERROR_0", "throw proj_exception(0)");
            boost::replace_all(line, "E_ERROR", "throw proj_exception");
            if (boost::contains(line, "throw") && ! boost::ends_with(line, ";"))
            {
                line += ";";
            }

            replace_return(line, "P");

            if (boost::contains(line, "par.fwd")
                || boost::contains(line, "par.inv")
                || boost::contains(line, "par.spc"))
            {
                line = tab1 + "// " + boost::trim_copy(line);
            }

            // Special cases
            if (projection_group == "sterea")
            {
                if (boost::contains(line, "pj_gauss_ini"))
                {
                    line = tab1 + "proj_parm.en = detail::gauss::gauss_ini(par.e, par.phi0, proj_parm.phic0, R);";
                }
            }
            else if (projection_group == "ob_tran")
            {
                boost::replace_all(line, "proj_parm.link->", "pj.");
            }

            if (use_mlfn())
            {
                if (boost::contains(boost::replace_all_copy(line, " ", ""), "en=pj_enfn"))
                {
                    line = tab2 + "pj_enfn(par.es, proj_parm.en);";
                    skip_lineno = lineno + 1;
                }
            }
            if (use_auth())
            {
                if (boost::contains(boost::replace_all_copy(line, " ", ""), "apa=pj_authset"))
                {
                    line = tab2 + "pj_authset(par.es, proj_parm.apa);";
                }
            }
            if (use_mdist())
            {
                if (boost::contains(boost::replace_all_copy(line, " ", ""), "en=proj_mdist_ini"))
                {
                    line = tab1 + "proj_mdist_ini(par.es, proj_parm.en);";
                    skip_lineno = lineno + 1;
                }
            }
        }

        void replace_macros(std::string& line)
        {
            // Replace all "defines" containing -> with the defined constant
            for (std::vector<define>::const_iterator it = defined_parameters.begin();
                it != defined_parameters.end(); it++)
            {
                boost::replace_all(line, it->name, it->value);
            }

            if (boost::contains(boost::replace_all_copy(line, " ", ""), "pj_errno="))
            {
                boost::replace_all(line, "pj_errno", "throw proj_exception(");
                boost::replace_all(line, "=", "");
                boost::replace_all(line, ";", ");");
                boost::replace_all(line, ";;", ";");
            }
        }


        void replace_for_setup(std::string const& line, std::vector<std::string>& setup)
        {
            if (projection_group == "ob_tran")
            {
                std::string trimmed = boost::trim_copy(line);
                if (trimmed == "char *name, *s;")
                {
                    setup.push_back("");
                    skip_for_setup = true;
                    return;
                }
                else if (skip_for_setup && boost::starts_with(trimmed, "if (!s || !("))
                {
                    skip_for_setup = false;
                    setup.push_back("");
                    setup.push_back(tab1 + "Parameters pj;");
                    return;
                }
                else if (boost::contains(trimmed, "link = pj_list[i]"))
                {
                    skip_for_setup = true;
                    return;
                }
                else if (skip_for_setup && trimmed == "}")
                {
                    skip_for_setup = false;
                    setup.push_back(tab1 + "pj.name = pj_param(par.params, \"so_proj\").s;");
                    setup.push_back("");
                    setup.push_back(tab1 + "factory<Geographic, Cartesian, Parameters> fac;");
                    setup.push_back(tab1 + "if (create)");
                    setup.push_back(tab1 + "{");
                    setup.push_back(tab2 + "proj_parm.link.reset(fac.create_new(pj));");
                    setup.push_back(tab2 + "if (! proj_parm.link.get()) throw proj_exception(-26);");
                    setup.push_back(tab1 + "}");
                    return;
                }
                else if (skip_for_setup)
                {
                    return;
                }
            }
            else if (projection_group == "goode")
            {
                if (boost::contains(line, "!(P->sinu = pj_sinu")
                    || boost::contains(line, "E_ERROR_0"))
                {
                    return;
                }
            }

            std::vector<std::string> lines;
            split(line, lines, "", ";");
            for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++)
            {
                replace_macros(*it);
                replace_in_entry(*it);
                setup.push_back(*it);
            }
        }

        void parse_characteristics(std::string chars, derived& proj)
        {
            boost::replace_all(chars, "\\t", " ");
            boost::replace_all(chars, "\\n", ",");
            boost::replace_all(chars, "\"", "");
            ////boost::replace_all(chars, ";", " ");
            //boost::replace_all(chars, ",", " ");
            //boost::replace_all(chars, "&", " ");
            boost::replace_all(chars, ".", " ");
            boost::replace_all(chars, "  ", " ");

            //boost::replace_all(chars, "no inv", "noinv");
            //boost::replace_all(chars, "new pole", "newpole");
            boost::replace_all(chars, "Sph&Ell", "Sph,Ell");
            boost::replace_all(chars, "Misc Sph", "Misc,Sph");
            boost::replace_all(chars, "PCyl Sph", "PCyl,Sph");
            boost::replace_all(chars, "Conic Sph", "Conic,Sph");

            proj.characteristics += " " + chars;

            proj.parsed_characteristics.clear();
            split(proj.characteristics, proj.parsed_characteristics, ",;\n");
            for (unsigned int i = 0; i < proj.parsed_characteristics.size(); i++)
            {
                std::string ch = boost::trim_copy(proj.parsed_characteristics[i]);
                if (ch == "no inv")
                {
                    ch = "no inverse";
                }
                else if (ch == "Sph" || ch == "Spheroid")
                {
                    ch = "Spheroid";
                }
                else if (ch == "Ell" || ch == "Ellipsoid")
                {
                    ch = "Ellipsoid";
                }
                else if (ch == "Azi" || ch == "Azimuthal")
                {
                    ch = "Azimuthal";
                }
                else if (boost::iequals(ch, "PCyl"))
                {
                    ch = "Pseudocylindrical";
                }
                else if (ch == "Conic")
                {
                }
                else if (ch == "Misc")
                {
                    ch = "Miscellaneous";
                }
                else if (ch == "Cyl")
                {
                    ch = "Cylindrical";
                }
                else if (boost::contains(ch, "="))
                {
                    //boost::replace_all(ch, "=", "");
                    //ch = "parameter " + ch;
                }
                else if (! ch.empty())
                {
                    std::cerr << projection_group << " par " << ch << std::endl;
                }
                proj.parsed_characteristics[i] = ch;
            }
        }

        std::string get_model(std::string const& line, std::string const& tag)
        {
            std::string model = line;
            boost::replace_all(model, boost::to_upper_copy(tag), "");
            boost::replace_all(model, "(", "");
            boost::replace_all(model, ")", "");
            boost::replace_all(model, ";", "");

            strip_comments(model);
            boost::trim(model);

            if (model == "e_" + tag) model = "ellipsoid";
            else if (model == "s_" + tag) model = "spheroid";
            else if (model == "o_" + tag) model = "oblique";
            else if (model == "t_" + tag) model = "transverse";
            else if (model == "e_guam_fwd" || model == "e_guam_inv") model = "guam";
            else if (model == tag) model = "other";
            else std::cerr << "ERROR IN MODEL " << model << std::endl;

            return model;
        }


        void parse(std::string const& filename)
        {
            std::ifstream cpp_file(filename.c_str());

            bool in_projection = false;
            bool in_constructor = false;
            bool in_prefix = true;
            bool in_postfix = false;
            bool in_define = false;
            bool in_comment = false;
            bool in_proj_params = false;

            static std::string PROJ_HEAD("PROJ_HEAD(");

            std::vector<projection>::iterator proj_it;
            std::vector<derived>::iterator derived_it;

            bool in_special_case = false;
            int proj_lineno = -1;


            if (cpp_file.is_open())
            {
                while (! cpp_file.eof() )
                {

                    std::string line;
                    std::getline(cpp_file, line);
                    lineno++;

                    // Avoid tabs
                    boost::replace_all(line, "\t", tab1);
                    boost::replace_all(line, "°", " DEG");

                    std::string trimmed = boost::trim_copy(line);

                    std::string model, direction;
                    std::vector<std::string> extra_entries;

                    bool started = false;

                    std::string::size_type loc;

                    if (find(line, PROJ_HEAD, loc)
                        || (lineno == proj_lineno + 1
                            && (boost::starts_with(trimmed, "\"") || boost::starts_with(trimmed, "LINE2"))
                            )

                        )
                    {
                        if (loc == std::string::npos
                            && lineno == proj_lineno + 1 && derived_projections.size() > 0)
                        {
                            if (projection_group == "sconics")
                            {
                                boost::replace_all(trimmed, "LINE2", "\n\tConic, Sph\n\tlat_1= and lat_2=");
                            }
                            parse_characteristics(trimmed, derived_projections.back());
                        }
                        else
                        {
                            loc += PROJ_HEAD.length();
                            std::string::size_type comma = line.find(',');
                            std::string::size_type end = line.find("\")", comma);
                            if (comma != std::string::npos && end != std::string::npos)
                            {
                                derived proj;
                                proj.name = line.substr(loc, comma - loc);
                                proj.description = boost::trim_copy(line.substr(comma + 1, end - comma));
                                std::string rest = line.substr(end + 2);

                                // Replace enclosing double quotes
                                std::string::size_type begin = proj.description.find('"');
                                end = proj.description.rfind('"');
                                if (begin != std::string::npos && end != std::string::npos
                                    && begin != end)
                                {
                                    proj.description.erase(end);
                                    proj.description.erase(begin, 1);
                                }

                                parse_characteristics(rest, proj);

                                derived_projections.push_back(proj);
                            }
                        }
                        proj_lineno = lineno;
                    }
                    else if (boost::starts_with(trimmed, "FORWARD"))
                    {
                        in_prefix = false;
                        in_projection = true;
                        started = true;
                        direction = "forward";

                        model = get_model(trimmed, direction);
                    }
                    else if (boost::starts_with(trimmed, "INVERSE"))
                    {
                        in_prefix = false;
                        in_projection = true;
                        started = true;
                        direction = "inverse";

                        model = get_model(trimmed, direction);
                    }
                    else if (boost::starts_with(trimmed, "SPECIAL"))
                    {
                        in_prefix = false;
                        in_projection = true;
                        started = true;
                        direction = "special_factors";  // pseudo. Is in lcc and in eqdc

                        model = proj_it->model; // do not change the model
                    }
                    else if (projection_group == "imw_p" && trimmed == "static void")
                    {
                        // Special case, function "xy" following normal implementation, add to prefix
                        prefix.push_back("");
                        prefix.push_back("inline void");

                        in_prefix = true;
                        in_projection = false;
                        in_special_case = true;
                        started = true;

                    }
                    else if (projection_group == "imw_p" && in_special_case && trimmed == "}")
                    {
                        // End special case, see comment above
                        in_special_case = false;
                        in_prefix = false;
                        prefix.push_back("}");
                        prefix.push_back("");
                    }
                    else if (boost::starts_with(trimmed, "FREEUP"))
                    {
                        in_projection = false;
                        started = true;
                    }
                    else if (boost::starts_with(trimmed, "ENTRY"))
                    {
                        in_constructor = true;
                        in_postfix = false;
                        started = true;

                        std::string entry = trimmed.substr(6);
                        std::string name = entry;

                        // Some files contain all initialization in one line. So split it here
                        if (boost::contains(entry, "ENDENTRY"))
                        {
                            // Insert extra ; to split easier
                            std::string::size_type pos = entry.find(')');
                            if (pos != std::string::npos)
                            {
                                entry.insert(pos + 1, ";");
                            }
                            //boost::split(extra_entries, entry, boost::is_any_of(";"));
                            split(entry, extra_entries, ";");

                            if (extra_entries.size() > 0)
                            {
                                name = extra_entries[0];
                                extra_entries.erase(extra_entries.begin());
                            }
                            started = false;
                            in_constructor = false;
                        }

                        boost::replace_all(name, "(", "");
                        boost::replace_all(name, ")", "");
                        boost::replace_all(name, ";", "");
                        // ENTRY1 entries have a comma plus "en". Remove that.
                        std::string::size_type loc = name.find(",");
                        if (loc != std::string::npos)
                        {
                            name.erase(loc);
                        }


                        // Find corresponding projection
                        derived_it = derived_projections.begin();
                        while (derived_it != derived_projections.end() && derived_it->name != name)
                        {
                            derived_it++;
                        }
                        if (derived_it == derived_projections.end())
                        {
                            std::cerr << "WARNING: " << "ENTRY: " << name << " not found" << std::endl;
                        }


                    }
                    else if (boost::starts_with(trimmed, "ENDENTRY"))
                    {
                        // Some files have setup(...) here
                        std::string parameters = end_entry(trimmed);
                        if (boost::contains(parameters, "setup") && derived_it != derived_projections.end())
                        {
                            boost::replace_all(trimmed, "ENDENTRY(", "");
                            boost::replace_last(trimmed, ")", "");
                            boost::replace_first(trimmed, "setup(P", "setup(par");
                            derived_it->constructor_lines.push_back(tab1 + trimmed + ";");
                        }
                        in_constructor = false;
                    }
                    else if (boost::starts_with(trimmed, "setup") && setup_function_line.empty())
                    {
                        in_postfix = true;
                        setup_function_line = trimmed;
                        boost::replace_all(setup_function_line, "PJ *P", "Parameters& par");
                        boost::replace_all(setup_function_line, "static PJ *", "void");
                        /*libproject*/boost::replace_all(setup_function_line, "PROJ *P", "Parameters& par");
                        /*libproject*/boost::replace_all(setup_function_line, "static PROJ *", "void");
                        boost::replace_all(setup_function_line, "{", "");

                        // add void (in most/all cases 'static PJ *' was on the previous line)
                        if (! boost::starts_with(setup_function_line, "void"))
                        {
                            setup_function_line = "void " + setup_function_line;
                        }

                    }
                    else if (boost::contains(trimmed, "PROJ_PARMS"))
                    {
                        in_proj_params = true;
                    }
                    else if (boost::starts_with(boost::replace_all_copy(trimmed, " ", ""), "#define"))
                    {
                        in_proj_params = false;
                        if (boost::ends_with(trimmed, "\\"))
                        {
                            in_define = true;
                        }
                        else
                        {
                            typedef boost::tokenizer<boost::char_separator<char> > TOK;
                            TOK tokens(trimmed, boost::char_separator<char>(" #\t"));
                            std::vector<std::string> words;
                            for (TOK::iterator it = tokens.begin(); it != tokens.end(); it++)
                            {
                                if (! (*it == "define"))
                                {
                                    words.push_back(*it);
                                }
                            }

                            if (words.size() == 2)
                            {
                                define d;
                                d.name = words[0];
                                d.value = words[1];
                                if (boost::contains(d.value, "->"))
                                {
                                    defined_parameters.push_back(d);
                                }
                                else
                                {
                                    defined_consts.push_back(d);
                                }
                            }
                            else if (! (boost::contains(trimmed, "PJ_LIB_")
                                        || words[0] == "LINE2"
                                        || words[0] == "TSFN0(x)"
                                        || words[0] == "V(C,z)"
                                        || words[0] == "DV(C,z)"
                                        ))
                            {
                                std::cerr << "Ignored: " << words[0] << "-" << trimmed << std::endl;
                            }
                        }
                    }
                    else if (boost::starts_with(boost::replace_all_copy(trimmed, " ", ""), "#define"))
                    {
                        in_proj_params = false;
                    }
                    else if (in_proj_params)
                        //&& (boost::contains(trimmed, "\\") || boost::ends_with(trimmed, ";") ) )
                    {
                        std::string par = trimmed;
                        boost::trim_right_if(par, boost::is_any_of("\\ "));
                        boost::replace_all(par, "\t", " ");

                        std::string par_constructor;
                        if (projection_group == "sterea")
                        {
                            boost::replace_all(par, "void *", "gauss::GAUSS ");
                        }
                        else if (use_mdist())
                        {
                            boost::replace_all(par, "void *", "MDIST ");
                        }
                        else if (projection_group == "chamb")
                        {
                            boost::replace_all(par, "XY", "CXY");
                        }
                        else if (projection_group == "goode")
                        {
                            if (boost::contains(trimmed, "struct PJconsts"))
                            {
                                if (boost::contains(trimmed, "sinu"))
                                {
                                    boost::replace_all(par, "struct PJconsts", "sinu_ellipsoid<Geographic, Cartesian, Parameters>");
                                }
                                else if (boost::contains(trimmed, "moll"))
                                {
                                    boost::replace_all(par, "struct PJconsts", "moll_spheroid<Geographic, Cartesian, Parameters>");
                                }
                                boost::replace_all(par, "*", "");
                            }
                            if (boost::contains(par, "moll"))
                            {
                                par_constructor = "par_goode(const Parameters& par) : sinu(par), moll(par) {}";
                            }
                        }

                        if (use_mlfn())
                        {
                            if (boost::contains(trimmed, "double")
                                && boost::contains(trimmed, "*en"))
                            {
                                boost::replace_all(par, "*en", "en[EN_SIZE]");
                            }
                        }
                        if (use_auth())
                        {
                            if (boost::contains(trimmed, "double")
                                && boost::contains(trimmed, "*apa"))
                            {
                                boost::replace_all(par, "*apa", "apa[APA_SIZE]");
                            }
                        }

                        boost::replace_all(par, "struct PJconsts *", "boost::shared_ptr<projection<Geographic, Cartesian> > ");

                        proj_parameters.push_back(par);

                        if (! boost::contains(trimmed, "\\"))
                        {
                            in_proj_params = false;
                        }
                        proj_parameter_names = extract_names(proj_parameters);

                        if (! par_constructor.empty())
                        {
                            proj_parameters.push_back("");
                            proj_parameters.push_back(par_constructor);
                        }


                    }
                    else if (in_prefix
                        && ! boost::contains(trimmed, "*/")
                        && (boost::starts_with(trimmed, "/***")
                            || boost::starts_with(trimmed, "/* PROJ.4")
                            || trimmed == "/*"
                            )
                        )
                    {
                        in_comment = true;
                    }
                    else if (in_comment && boost::contains(trimmed, "*/"))
                    {
                        in_comment = false;
                    }
                    else if (in_prefix
                        && ! in_comment
                        && ! in_define
                        && ! boost::starts_with(trimmed, "#") // define,include,ifdef,endif
                        && ! boost::contains(line, "SCCSID[]")
                        && ! boost::contains(line, "PJ_CVSID")
                        && ! boost::contains(line, "RCS_ID[]")
                        && ! boost::contains(line, "LIBPROJ_ID")
                        && trimmed != std::string("static const char") // libproj_id often splitted on two lines
                        && trimmed != std::string("LINE2;") //in pj_sconics
                        && ! (boost::starts_with(trimmed, "\"") && boost::ends_with(trimmed, "\";")) // after projhead
                        && ! (boost::starts_with(trimmed, "\"") && boost::ends_with(trimmed, "\""))) // after projhead
                    {

                        bool skip = false;

                        std::string par = "par_" + projection_group;



                        // Special cases
                        if (projection_group == "chamb")
                        {
                            if (boost::contains(line, "typedef struct") && boost::contains(line, "VECT"))
                            {
                                skip = true;
                            }
                        }
                        else if (projection_group == "robin")
                        {
                            boost::replace_all(line, "float c0, c1", "double c0, c1");
                            // Add missing braces, all values are separated by , and have a ., the rest
                            // of the lines here do not have that combination
                            if (boost::contains(line, ".") && boost::contains(line, ","))
                            {
                                boost::trim(line);

                                // add begin brace
                                line.insert(0, "{");
                                if (boost::contains(line, "}"))
                                {
                                    // For the last line: add double end brace
                                    boost::replace_last(line, "}", "}}");
                                }
                                else
                                {
                                    // The normal case: add end brace
                                    boost::replace_last(line, ",", "},");
                                }
                            }
                        }
                        else if (projection_group == "goode")
                        {
                            if (boost::contains(line, "C_NAMESPACE")
                                || boost::contains(line, "*pj_sinu(PJ *)")
                                || boost::contains(line, "*pj_sinu(PROJ *)")
                                )
                            {
                                line.clear();
                                skip = true;
                            }
                        }
                        else if (projection_group == "sconics"
                            || projection_group == "lsat"
                            || projection_group == "imw_p"
                            )
                        {
                            // imw_p:
                            boost::replace_all(line, " XY", " PXY");

                            std::string tag = "PJ *P";
                            std::string::size_type loc = line.find(tag);
                            if (loc == std::string::npos)
                            {
                                // libproject
                                tag = "PROJ *P";
                                loc = line.find(tag);
                            }

                            if (loc != std::string::npos)
                            {
                                line.insert(loc + tag.length(),
                                        ", " + par + "& proj_parm");

                                // imw_p:
                                if (boost::contains(line, "LP lp"))
                                {
                                    boost::replace_all(line, "LP lp", "double const& lp_lam, double const& lp_phi");
                                    boost::replace_all(line, "PJ *", "const PJ *"); // not yet replaced here
                                    boost::replace_all(line, "par_imw_p&", "par_imw_p const&");
                                }
                            }

                            // imw_p:
                            boost::replace_all(line, "lp.lam", "lp_lam");
                            boost::replace_all(line, "lp.phi", "lp_phi");
                            boost::replace_all(line, " XY xy", " PXY xy");
                        }


                        // Change static keyword in function to inline keyword. NOT FOR STATIC DATA...
                        if (boost::starts_with(trimmed, "static "))
                        {
                            if (projection_group == "nzmg"
                                || projection_group == "robin" )
                            {
                                //boost::replace_all(line, "static ", "static const ");
                            }
                            else
                            {
                                boost::replace_all(line, "static ", "inline ");
                            }
                        }

                        if (! skip)
                        {
                            if (boost::contains(line, "PJ *P") && prefix.size() > 0)
                            {
                                // Add a template line before the line before
                                std::string last = prefix.back();
                                prefix.pop_back();
                                prefix.push_back("template <typename Parameters>");
                                prefix.push_back(last);
                            }

                            boost::replace_all(line, "PJ *P", "Parameters& par");
                            replace_pars(line);
                            prefix.push_back(line);
                        }
                    }
                    else if (in_postfix)
                    {
                        replace_for_setup(line, postfix);
                    }
                    else
                    {
                        in_proj_params = false;
                    }

                    if (! boost::ends_with(trimmed, "\\"))
                    {
                        in_define = false;
                    }

                    // Process projection functions
                    if (in_projection)
                    {
                        if (started)
                        {
                            projection proj;
                            proj.model = model;
                            proj.direction = direction;
                            //proj.lines.push_back("\t//Model: " + model + ", Direction: " + direction);
                            projections.push_back(proj);

                            proj_it = projections.begin() + (projections.size() - 1);

                        }
                        else if (proj_it != projections.end())
                        {
                            replace_macros(line);
                            replace_pars(line, "this->m_");

                            replace_return(line, "xy");
                            replace_return(line, "lp");

                            boost::replace_all(line, "M_PI_2", "(2.0 * boost::math::constants::pi<double>())");
                            boost::replace_all(line, "M_PI", "boost::math::constants::pi<double>()");

                            boost::replace_all(line, "hypot", "boost::math::hypot");
                            // BEGIN libproject:
                            boost::replace_all(line, "proj_asin", "std::asin");
                            boost::replace_all(line, "proj_acos", "std::acos");
                            // END libproject

                            boost::replace_all(line, "xy.x", "xy_x");
                            boost::replace_all(line, "xy.y", "xy_y");
                            boost::replace_all(line, "lp.lam", "lp_lon");
                            boost::replace_all(line, "lp.phi", "lp_lat");

                            boost::replace_all(line, "F_ERROR", "throw proj_exception();");
                            boost::replace_all(line, "I_ERROR", "throw proj_exception();");

                            std::string line2;

                            // Special cases
                            if (projection_group == "sterea")
                            {
                                if (boost::starts_with(boost::trim_copy(line), "lp = pj_gauss("))
                                {
                                    line = tab1 + "detail::gauss::gauss(m_proj_parm.en, lp_lon, lp_lat);";
                                }
                                else if (boost::contains(line, "return(pj_inv_gauss"))
                                {
                                    line = tab1 + "detail::gauss::inv_gauss(m_proj_parm.en, lp_lon, lp_lat);";
                                }
                            }
                            else if (projection_group == "mod_ster")
                            {
                                boost::replace_all(line,
                                    "double den, rh, z, sinz, cosz, chi, phi",
                                    "double den, rh = 0, z, sinz = 0, cosz = 0, chi, phi = 0");

                            }
                            else if (projection_group == "chamb")
                            {
                                std::string s = boost::trim_copy(line);
                                if (boost::starts_with(s, "xy ="))
                                {
                                    boost::replace_all(s, "xy =", "");
                                    boost::trim(s);
                                    boost::replace_last(s, ";", "");
                                    line = tab2 + "{ xy_x = " + s + ".x; xy_y = " + s + ".y; }";
                                }
                            }
                            else if (projection_group == "ob_tran")
                            {
                                std::string copy = boost::trim_copy(line);
                                if (boost::starts_with(copy, "return (this->m_proj_parm.link->fwd"))
                                {
                                    line = tab1 + "m_proj_parm.link->fwd(lp_lon, lp_lat, xy_x, xy_y);";
                                }
                                else if (boost::starts_with(copy, "lp = this->m_proj_parm.link->inv"))
                                {
                                    line = tab1 + "m_proj_parm.link->inv(xy_x, xy_y, lp_lon, lp_lat);";
                                }
                                else if (copy == "(void) xy;")
                                {
                                    line = "";
                                }
                            }
                            else if (projection_group == "imw_p")
                            {
                                boost::replace_all(line, "XY t", "PXY t");
                                if (boost::contains(line, "loc_for(lp"))
                                {
                                    // replace arguments
                                    boost::replace_all(line, "lp, P, &yc", "lp_lon, lp_lat, this->m_par, m_proj_parm, &yc");
                                    // replace return type
                                    boost::replace_all(line, "xy =", "PXY xy =");
                                    if (boost::contains(line, "PXY"))
                                    {
                                        line2 = tab1 + "xy_x = xy.x; xy_y = xy.y;";
                                    }
                                }
                            }
                            else if (projection_group == "goode")
                            {
                                std::string s = boost::trim_copy(line);
                                if (boost::starts_with(s, "xy =") || boost::starts_with(s, "lp ="))
                                {
                                    // 4 cases will become "m_proj_parm.sinu.inv(xy_x, xy_y, lp_lon, lp_lat);" or similar
                                    boost::replace_all(line, "xy = ", "");
                                    boost::replace_all(line, "lp = ", "");
                                    boost::replace_last(line, "this->m_proj_parm.sinu", "");
                                    boost::replace_last(line, "this->m_proj_parm.moll", "");
                                    boost::replace_all(line, "lp, ", "lp_lon, lp_lat, xy_x, xy_y");
                                    boost::replace_all(line, "xy, ", "xy_x, xy_y, lp_lon, lp_lat");
                                    boost::replace_all(line, "->fwd", ".fwd");
                                    boost::replace_all(line, "->inv", ".inv");
                                }
                            }

                            // Unused variables
                            if (projection_group == "krovak")
                            {
                                boost::replace_all(line, "char errmess[255];", "");
                                boost::replace_all(line, "char tmp[16];", "");
                                boost::replace_all(line, ", lon17", "");
                                boost::replace_all(line, ", lamdd", "");
                                boost::replace_all(line, ", l24", "");
                            }
                            else if (projection_group == "geos")
                            {
                                boost::replace_all(line, ", c", "");
                            }
                            else if (projection_group == "gn_sinu")
                            {
                                boost::replace_all(line, "double s;", "double s; boost::ignore_unused_variable_warning(s);");
                            }
                            else if (projection_group == "lcca")
                            {
                                boost::replace_all(line, ", S3", "");
                            }
                            else if (projection_group == "mbt_fps")
                            {
                                boost::replace_all(line, ", s", "");
                            }
                            else if (projection_group == "moll" || projection_group == "nell")
                            {
                                boost::replace_all(line, "double th, s;", "");
                            }

                            // uninitialized variables warning
                            if (projection_group == "imw_p")
                            {
                                boost::replace_all(line, "double yc;", "double yc = 0;");
                            }



                            proj_it->lines.push_back(line);
                            if (! line2.empty())
                            {
                                proj_it->lines.push_back(line2);
                            }
                        }
                    }
                    else if (in_constructor)
                    {
                        if (! started && derived_it != derived_projections.end())
                        {
                            // Replace call to "setup(P)" with appropriate
                            if (boost::starts_with(trimmed, "setup(P"))
                            {
                                    boost::replace_first(line, "(P", "(par");
                            }
                            replace_for_setup(line, derived_it->constructor_lines);
                        }
                    }
                    else if (extra_entries.size() > 0 && derived_it != derived_projections.end())
                    {
                        for (std::vector<std::string>::iterator it = extra_entries.begin(); it != extra_entries.end(); it++)
                        {
                            if (boost::contains(*it, "ENDENTRY"))
                            {
                                std::string par = end_entry(*it);
                                if (! par.empty())
                                {
                                    derived_it->constructor_lines.push_back(tab1 + par  + ";");
                                }
                            }
                            else
                            {
                                replace_in_entry(*it);
                                derived_it->constructor_lines.push_back(tab1 + boost::trim_copy(*it + ";"));
                            }
                        }
                    }

                    //stream << line << std::endl;
                }
                cpp_file.close();
            }
        }

        void write_copyright()
        {
            std::ifstream cr_file ("proj4_to_boost_geometry_copyright_header.txt");
            if (cr_file.is_open())
            {
                while (! cr_file.eof() )
                {
                    std::string line;
                    std::getline(cr_file, line);

                    stream << line << std::endl;
                }
                cr_file.close();
            }
            stream << std::endl;
        }
        void write_header()
        {
            if (projection_group == "chamb")
            {
                stream << "#include <cstdio>" << std::endl;
                stream << std::endl;
            }


            if (projection_group == "ob_tran")
            {
                stream << "#include <boost/shared_ptr.hpp>" << std::endl;
            }

            if (use_ignore_unused())
            {
                stream
                    << "#include <boost/concept_check.hpp>" << std::endl;
            }

            stream
                << "#include <boost/math/special_functions/hypot.hpp>" << std::endl
                << std::endl;

            stream
                << include_projections << "/impl/base_static.hpp>" << std::endl
                << include_projections << "/impl/base_dynamic.hpp>" << std::endl
                << include_projections << "/impl/projects.hpp>" << std::endl
                << include_projections << "/impl/factory_entry.hpp>" << std::endl
                ;


            if (projection_group == "robin"
                || projection_group == "tmerc")
            {
                stream << include_projections << "/impl/function_overloads.hpp>" << std::endl;
            }

            if (projection_group == "sterea")
            {
                stream << include_projections << "/impl/pj_gauss.hpp>" << std::endl;
            }

            if (projection_group == "mod_ster")
            {
                stream << include_projections << "/impl/pj_zpoly1.hpp>" << std::endl;
            }

            if (projection_group == "chamb"
                || projection_group == "aeqd")
            {
                stream << include_projections << "/impl/aasincos.hpp>" << std::endl;
            }


            if (projection_group == "aea"
                || projection_group == "eqdc"
                || projection_group == "poly"
                || projection_group == "lcc"
                || projection_group == "merc"
                )
            {
                stream << include_projections << "/impl/pj_msfn.hpp>" << std::endl;
            }

            if (projection_group == "aea"
                || projection_group == "cea"
                || projection_group == "laea")
            {
                stream << include_projections << "/impl/pj_qsfn.hpp>" << std::endl;
            }

            if (projection_group == "merc"
                || projection_group == "omerc"
                || projection_group == "stere"
                || projection_group == "gstmerc"
                || projection_group == "lcc"
                )
            {
                stream << include_projections << "/impl/pj_tsfn.hpp>" << std::endl;
            }

            if (use_mlfn())
            {
                stream << include_projections << "/impl/pj_mlfn.hpp>" << std::endl;
            }

            if (use_auth())
            {
                stream << include_projections << "/impl/pj_auth.hpp>" << std::endl;
            }

            if (projection_group == "gstmerc"
                || projection_group == "lcc"
                || projection_group == "merc"
                || projection_group == "omerc")
            {
                stream << include_projections << "/impl/pj_phi2.hpp>" << std::endl;
            }


            if (use_mdist())
            {
                stream << include_projections << "/impl/proj_mdist.hpp>" << std::endl;
            }

            if (projection_group == "goode")
            {
                stream << std::endl;
                stream << include_projections << "/proj/moll.hpp>" << std::endl;
                stream << include_projections << "/proj/gn_sinu.hpp>" << std::endl;
            }

            if (use_epsg())
            {
                stream << std::endl;
                stream << include_projections << "/epsg_traits.hpp>" << std::endl;
            }

            stream << std::endl;

            stream << "namespace boost { namespace geometry { namespace projection" << std::endl
                << "{" << std::endl;

            if (projection_group == "ob_tran")
            {
                stream << std::endl
                    << tab1 << "template <typename Geographic, typename Cartesian, typename Parameters> class factory;"
                    << std::endl << std::endl;

            }

        }

        void write_begin_impl()
        {

            stream
                << tab1 << "#ifndef DOXYGEN_NO_DETAIL" << std::endl
                << tab1 << "namespace detail { namespace " << projection_group << "{ " << std::endl;
        }

        void write_proj_par_struct()
        {
            if (proj_parameters.size() > 0)
            {
                // for chamb:
                if (projection_group == "chamb")
                {
                    stream
                        << tab3 << "struct VECT { double r, Az; };" << std::endl
                        << tab3 << "struct CXY { double x, y; }; // x/y for chamb" << std::endl
                        << std::endl;
                }
                else if (projection_group == "imw_p")
                {
                    stream
                        << tab3 << "struct PXY { double x, y; }; // x/y projection specific" << std::endl
                        << std::endl;
                }

                std::string ts = template_struct();
                if (! ts.empty())
                {
                    stream << tab3 << "template <";
                    if (ts == "<Cartesian>")
                    {
                        stream  << "typename Cartesian";
                    }
                    else if (ts == "<Geographic, Cartesian>")
                    {
                        stream << "typename Geographic, typename Cartesian";
                    }
                    else if (ts == "<Geographic, Cartesian, Parameters>")
                    {
                        stream << "typename Geographic, typename Cartesian, typename Parameters";
                    }
                    stream << ">" << std::endl;
                }

                stream
                    << tab3 << "struct par_" << projection_group << std::endl
                    << tab3 << "{" << std::endl;
                for (std::vector<std::string>::const_iterator it = proj_parameters.begin(); it != proj_parameters.end(); it++)
                {
                    stream << tab4  << *it << std::endl;
                }
                stream << tab3 << "};" << std::endl;
            }
        }

        void write_consts()
        {
            for (std::vector<define>::const_iterator it = defined_consts.begin(); it != defined_consts.end(); it++)
            {
                std::string type = "int";
                if (boost::contains(it->value, ".") || boost::contains(it->value, "e"))
                {
                    type = "double";
                }
                stream << tab3 << "static const " << type << " " << it->name << " = " << it->value << ";" << std::endl;
            }
            stream << std::endl;

            if (projection_group == "omerc")
            {
                // Put macrodef, which was skipped, here as inline function
                stream << tab4 << "inline double TSFN0(double x) " << std::endl
                    << tab5 << "{return tan(.5 * (HALFPI - (x))); }" << std::endl
                    << std::endl << std::endl;

            }

        }

        void write_prefix()
        {
            for (std::vector<std::string>::const_iterator it = prefix.begin(); it != prefix.end(); it++)
            {
                stream << tab3 << *it << std::endl;
            }
            stream << std::endl;
        }

        void write_postfix()
        {
            if (postfix.size() > 0 && ! setup_function_line.empty() )
            {
                std::cerr << "SETUP " << projection_group << std::endl;
                if (proj_parameters.size() > 0)
                {
                    // Modify the setup function: add project parameter
                    std::string tag = "Parameters& par";
                    std::string::size_type loc = setup_function_line.find(tag);
                    if (loc != std::string::npos)
                    {
                        setup_function_line.insert(loc + tag.length(),
                                ", par_" + projection_group + "& proj_parm");
                    }
                }

                stream
                    << tab3 << "template <typename Parameters>" << std::endl
                    << tab3 << setup_function_line << std::endl
                    << tab3 << "{" << std::endl;

                if (use_ignore_unused())
                {
                    stream
                        << tab4 << "boost::ignore_unused_variable_warning(par);"
                        << std::endl;

                    if (proj_parameters.size() > 0)
                    {
                        stream
                            << tab4 << "boost::ignore_unused_variable_warning(proj_parm);"
                            << std::endl;
                    }
                }

                for (size_t j = 0; j < postfix.size(); j++)
                //for (std::vector<std::string>::const_iterator it = postfix.begin(); it != postfix.end(); it++)
                {
                    std::string const& line = postfix[j];
                    if (! (j == postfix.size() - 2 && boost::trim_copy(line) == "return;"))
                    {
                        stream << tab3 << line << std::endl;
                    }
                }
                stream << std::endl << std::endl;
            }
        }

        void write_impl_classes()
        {
            std::sort(projections.begin(), projections.end());

            // Build map of projections having the inverse operation
            for (size_t i = 0; i < projections.size(); i++)
            {
                projection const& proj = projections[i];
                if (proj.direction == "inverse")
                {
                    has_inverse[proj.model] = true;
                }
            }

            // Write classes

            std::string current_model;
            for (size_t i = 0; i < projections.size(); i++)
            {
                projection const& proj = projections[i];


                if (proj.model != current_model)
                {
                    models.push_back(proj.model);

                    std::string name = "base_" + projection_group + "_" + proj.model;

                    std::string tbase = "base_t_f";
                    std::map<std::string, bool>::iterator it = has_inverse.find(proj.model);
                    if (it != has_inverse.end() && it->second)
                    {
                        tbase += "i"; // base_fi
                    }

                    tbase += "<" + name + "<Geographic, Cartesian, Parameters>,"
                        + "\n" + tab5 + " Geographic, Cartesian, Parameters>";


                    std::string mut;
                    if (projection_group == "lcc"
                        || projection_group == "aea"
                        || projection_group == "cass"
                        || projection_group == "eqdc"
                        )
                    {
                        mut = "mutable ";
                    }

                    stream
                        << tab3 << "// template class, using CRTP to implement forward/inverse" << std::endl
                        << tab3 << "template <typename Geographic, typename Cartesian, typename Parameters>" << std::endl
                        << tab3 << "struct " << name << " : public " << tbase
                        << std::endl
                        << tab3 << "{" << std::endl << std::endl;

                    // for GCC (probably standard) typedefs again are necessary
                    stream
                        //<< tab4 << "typedef typename " << tbase << "::geographic_type geographic_type;" << std::endl
                        //<< tab4 << "typedef typename " << tbase << "::cartesian_type cartesian_type;" << std::endl
                        << tab4 << " typedef double geographic_type;" << std::endl
                        << tab4 << " typedef double cartesian_type;" << std::endl
                        << std::endl;


                    // optional project specific parameter variable
                    if (proj_parameters.size() > 0)
                    {
                        stream << tab4 << mut << "par_" << projection_group
                            << template_struct() << " m_proj_parm;" << std::endl;
                    }

                    stream << std::endl
                        // constructor
                        << tab4 << "inline " << name << "(const Parameters& par)" << std::endl
                        << tab5 << ": " << tbase << "(*this, par)"
                        << (projection_group == "goode" ? ", m_proj_parm(par)" : "") << " {}"
                        << std::endl << std::endl
                        ;
                }

                if (proj.model != current_model)
                {
                    if (projection_group == "robin")
                    {
                        stream << tab4 << "inline double  V(COEFS const& C, double z) const" << std::endl
                            << tab4 << "{ return (C.c0 + z * (C.c1 + z * (C.c2 + z * C.c3))); }" << std::endl
                            << tab4 << "inline double DV(COEFS const& C, double z) const" << std::endl
                            << tab4 << "{ return (C.c1 + z * (C.c2 + C.c2 + z * 3. * C.c3)); }"
                            << std::endl << std::endl;
                    }
                }

                if (proj.direction == "special_factors")
                {
                    stream << tab4 << "#ifdef SPECIAL_FACTORS_NOT_CONVERTED" << std::endl;
                }


                stream << tab4 << "inline void ";
                if (proj.direction == "forward")
                {
                    stream << "fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y";
                }
                else if (proj.direction == "inverse")
                {
                    stream << "inv(cartesian_type& xy_x, cartesian_type& xy_y, geographic_type& lp_lon, geographic_type& lp_lat";
                }
                else if (proj.direction == "special_factors")
                {
                    stream
                        << "fac(Geographic lp, Factors &fac";
                }
                else
                {
                    stream << proj.direction << "(";
                }

                stream << ") const" << std::endl
                    << tab4 << "{" << std::endl;



                for (size_t j = 0; j < proj.lines.size(); j++)
                {
                    // Output functions, taking into account:
                    // 1. return on last line is surpressed
                    // 2. copy local variables to output variables
                    bool last_normal = j == proj.lines.size() - 2;
                    if (! (last_normal && boost::trim_copy(proj.lines[j]) == std::string("return;")))
                    {
                        stream << tab4 << proj.lines[j] << std::endl;
                    }


                }

                if (proj.direction == "special_factors")
                {
                    stream << tab4 << "#endif" << std::endl;
                }

#ifdef NO_CRTP
                if (proj.direction == "forward" || proj.direction == "inverse")
                {
                    stream << tab4 << "inline void " << proj.direction << "(const "
                        << (proj.direction == "forward"
                            ? "LL& lp, XY& xy"
                            : "XY& xy, LL& lp"
                            )
                        << ") const" << std::endl
                        << tab4 << "{" << std::endl
                        << tab5 << "pj_"
                        << (proj.direction == "forward" ? "fwd" : "inv")
                        << "(*this, m_par, "
                        << (proj.direction == "forward" ? "lp, xy" : "xy, lp")
                        << ");" << std::endl
                        /*
                        << tab5 << (proj.direction == "forward"
                            ? "geographic_type lp_lon = geometry::get<0>(lp), lp_lat = geometry::get<1>(lp);"
                            : "cartesian_type xy_x = geometry::get<0>(xy), xy_y = geometry::get<1>(xy);"
                            ) << std::endl
                        << tab5 << (proj.direction == "forward"
                            ? "cartesian_type xy_x, xy_y;"
                            : "geographic_type lp_lon, lp_lat;"
                            ) << std::endl
                        << tab5 << proj.direction << "(" << (proj.direction == "forward"
                                ? "lp_lon, lp_lat, xy_x, xy_y"
                                : "xy_x, xy_y, lp_lon, lp_lat"
                                )
                        << ");" << std::endl
                        << tab5 << "geometry::set<0>("
                        << (proj.direction == "forward" ? "xy, xy_x" : "lp, lp_lon") << ");" << std::endl
                        << tab5 << "geometry::set<1>("
                        << (proj.direction == "forward" ? "xy, xy_y" : "lp, lp_lat") << ");" << std::endl
                        ***/
                        << tab4 << "}" << std::endl << std::endl;
                }
#endif

                // End of class
                if (i == projections.size() - 1 || projections[i + 1].model != projections[i].model)
                {
                    stream << tab3 << "};" << std::endl;
                }
                stream << std::endl;

                current_model = proj.model;
            }
        }

        std::string template_struct() const
        {
            if (projection_group == "ob_tran")
            {
                return "<Geographic, Cartesian>";
            }
            else if (projection_group == "goode")
            {
                return "<Geographic, Cartesian, Parameters>";
            }
            return "";

        }

        bool use_mlfn() const
        {
            return projection_group == "aea"
                || projection_group == "aeqd"
                || projection_group == "bonne"
                || projection_group == "cass"
                || projection_group == "eqdc"
                || projection_group == "gn_sinu"
                || projection_group == "imw_p"
                || projection_group == "lcca"
                || projection_group == "poly"
                || projection_group == "tmerc"
                ;
        }
        bool use_auth() const
        {
            return projection_group == "cea"
                || projection_group == "laea"
                ;
        }

        bool use_mdist() const
        {
            return projection_group == "rouss"
                || projection_group == "aeqd"
                ;
        }

        bool use_epsg() const
        {
            for (std::vector<derived>::const_iterator dit = derived_projections.begin(); dit != derived_projections.end(); dit++)
            {
                for (std::vector<epsg_entry>::const_iterator eit = epsg_entries.begin(); eit != epsg_entries.end(); eit++)
                {
                    if (eit->prj_name == dit->name)
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        bool use_ignore_unused() const
        {
            return projection_group == "aea"
                || projection_group == "aitoff"
                || projection_group == "eck3"
                || projection_group == "gn_sinu"
                || projection_group == "mod_ster"
                || projection_group == "moll"
                || projection_group == "nsper"
                || projection_group == "putp3"
                || projection_group == "putp4p"
                || projection_group == "putp5"
                || projection_group == "putp6"
                || projection_group == "sconics"
                || projection_group == "stere"
                || projection_group == "sts"
                || projection_group == "tmerc"
                || projection_group == "urmfps";
        }

        void write_setup()
        {
            for (std::vector<derived>::const_iterator it = derived_projections.begin(); it != derived_projections.end(); it++)
            {

                stream
                    << tab3 << "// " << it->description << std::endl
                    << tab3 << "template <";
                std::string ts = template_struct();
                if (! ts.empty())
                {
                    stream << "typename ";
                    if (ts == "<Cartesian>") stream << "Cartesian";
                    else if (ts == "<Geographic, Cartesian>" || ts == "<Geographic, Cartesian, Parameters>") stream << "Geographic, typename Cartesian";
                    stream << ", ";
                }
                std::string return_type = "void";
                std::string extra_par = "";
                if (projection_group == "ob_tran")
                {
                    return_type = "double";
                    extra_par = ", bool create = true";
                }

                stream << "typename Parameters>" << std::endl
                    << tab3 << return_type << " setup_" << it->name << "(Parameters& par";

                if (proj_parameters.size() > 0)
                {
                    stream << ", par_" << projection_group << template_struct() << "& proj_parm";
                }
                stream << extra_par << ")" << std::endl
                    << tab3 << "{" << std::endl;

                for (size_t j = 0; j < it->constructor_lines.size(); j++)
                {
                    std::string line = it->constructor_lines[j];
                    if (boost::starts_with(boost::trim_copy(line), "setup")
                        && proj_parameters.size() > 0)
                    {
                        // Insert second parameter if necessary.
                        boost::replace_all(line, "par,", "par, proj_parm,");
                        boost::replace_all(line, "par)", "par, proj_parm)");
                    }
                    stream << tab3 << line << std::endl;
                }
                if (projection_group == "ob_tran")
                {
                    stream
                        << tab4 << "boost::ignore_unused_variable_warning(i);" << std::endl
                        << tab4 << "// return phip to choose model" << std::endl
                        << tab4 << "return phip;" << std::endl;
                }
                stream << tab3 << "}" << std::endl << std::endl;
            }
        }

        void write_end_impl()
        {
            stream << tab2 << "}} // namespace detail::" << projection_group << std::endl
                << tab1 << "#endif // doxygen " << std::endl
                << std::endl;
        }
        void write_end()
        {
            stream
                << "}}} // namespace boost::geometry::projection" << std::endl << std::endl
                << "#endif // " << hpp << std::endl << std::endl;
        }

        void write_classes()
        {
            for (std::vector<std::string>::const_iterator mit = models.begin(); mit != models.end(); mit++)
            {
                for (std::vector<derived>::const_iterator dit = derived_projections.begin(); dit != derived_projections.end(); dit++)
                {

                    std::string name = dit->name + "_" + *mit;
                    bool not_valid =
                            name == "ups_spheroid"
                            || name == "utm_spheroid"
                            || name == "gn_sinu_ellipsoid"
                            || name == "eck6_ellipsoid"
                            || name == "mbtfps_ellipsoid"

                            ;

                    if (! not_valid)
                    {
                        std::string base = "detail::" + projection_group
                            + "::base_" + projection_group + "_" + *mit + "<Geographic, Cartesian, Parameters>";

                        // Doxygen comments
                        stream
                            << tab1 << "/*!" << std::endl
                            << tab2 << "\\brief " << dit->description << " projection" << std::endl
                            << tab2 << "\\ingroup projections" << std::endl
                            << tab2 << "\\tparam Geographic latlong point type" << std::endl
                            << tab2 << "\\tparam Cartesian xy point type" << std::endl
                            << tab2 << "\\tparam Parameters parameter type" << std::endl
                            << tab2 << "\\par Projection characteristics" << std::endl
//#define ONE_CH
#ifdef ONE_CH
                            << tab2 << " - ALL:" << dit->characteristics << std::endl
#else
                        ;
                        for (std::vector<std::string>::const_iterator cit = dit->parsed_characteristics.begin();
                                cit != dit->parsed_characteristics.end(); cit++)
                        {
                            std::string ch = boost::trim_copy(*cit);
                            if (! ch.empty())
                            {
                                stream << tab2 << " - " << *cit << std::endl;
                            }
                        }
                        stream
#endif
                            << tab2 << "\\par Example" << std::endl
                            << tab2 << "\\image html ex_" << dit->name << ".gif" << std::endl
                            << tab1 << "*/" << std::endl;

                        // Class itself
                        stream
                            << tab1 << "template <typename Geographic, typename Cartesian, typename Parameters = parameters>" << std::endl
                            << tab1 << "struct " << name
                            << " : public " << base << std::endl
                            << tab1 << "{"  << std::endl
                            << tab2 << "inline " << name << "(const Parameters& par) : " << base << "(par)" << std::endl
                            << tab2 << "{" << std::endl
                            << tab3 << "detail::" << projection_group << "::setup_" << dit->name << "(this->m_par";
                        if (proj_parameters.size() > 0)
                        {
                            stream << ", this->m_proj_parm";
                        }
                        stream << ");" << std::endl
                            << tab2 << "}" << std::endl
                            << tab1 << "};" << std::endl
                            << std::endl;
                    }
                }
            }
        }

        void write_wrappers()
        {
            stream
                << tab1 << "#ifndef DOXYGEN_NO_DETAIL" << std::endl
                << tab1 << "namespace detail" << std::endl
                << tab1 << "{" << std::endl << std::endl;

            std::string templates = "template <typename Geographic, typename Cartesian, typename Parameters>";


            // TEMP: get model from below
            // TODO: get model from epsg-parameter
            std::string epsg_model = "";

            // Create factory entries
            // This complicated piece has
            // - to decide to take either "ellipsoid" or "spheroid" based on the input parameter
            // - to decide if it is the forward or forward/reverse model

            stream << tab2 << "// Factory entry(s)" << std::endl;
            for (std::vector<derived>::const_iterator dit = derived_projections.begin(); dit != derived_projections.end(); dit++)
            {
                stream << tab2 << "" << templates << std::endl
                    << tab2 << "class " << dit->name << "_entry : public detail::factory_entry<Geographic, Cartesian, Parameters>" << std::endl
                    << tab2 << "{" << std::endl
                    << tab3 << "public :" << std::endl
                    << tab4 << "virtual projection<Geographic, Cartesian>* create_new(const Parameters& par) const" << std::endl
                    << tab4 << "{" << std::endl;

                unsigned int m = 0;
                std::string tab = tab5;
                bool do_else = false;
                if (models.size() > 1)
                {
                    tab += tab1;
                    if (projection_group == "bonne"
                        || projection_group == "cass"
                        || projection_group == "cea"
                        || projection_group == "geos"
                        || projection_group == "laea"
                        || projection_group == "merc"
                        || projection_group == "poly"
                        || (projection_group == "stere" && dit->name == "stere")
                        || (projection_group == "tmerc" && dit->name == "tmerc")
                        || (projection_group == "gn_sinu" && dit->name == "sinu")
                        )
                    {
                        stream << tab5 << "if (par.es)" << std::endl;
                        do_else = true;
                    }
                    else if ((projection_group == "stere" && dit->name == "ups")
                        || (projection_group == "tmerc" && dit->name == "utm")
                        )
                    {
                        m++;
                        tab = tab5;
                    }
                    else if (projection_group == "gn_sinu") // gn_sinu,eck6, mbtfps
                    {
                        tab = tab5;
                    }
                    else if (projection_group == "aeqd")
                    {
                        std::reverse(models.begin(), models.end()); // -> spheroid, gaum, ellipsoid
                    }
                    else if (projection_group == "ob_tran")
                    {
                        stream << tab5 << "detail::ob_tran::par_ob_tran<Geographic, Cartesian> proj_parm;" << std::endl
                            << tab5 << "Parameters p = par;" << std::endl
                            << tab5 << "double phip = setup_ob_tran(p, proj_parm, false);" << std::endl
                            << tab5 << "if (fabs(phip) > detail::ob_tran::TOL)" << std::endl;
                        do_else = true;
                    }
                    else
                    {
                        stream << tab5 << "// TODO: check which model and return appropriate" << std::endl;
                    }
                }

                for (std::vector<std::string>::const_iterator mit = models.begin();
                    mit != models.end() && m < models.size();
                    mit++, m++)
                {
                    bool skip = false;
                    if (m == 1 && do_else)
                    {
                        stream << tab5 << "else" << std::endl;
                    }
                    else if (projection_group == "aeqd")
                    {
                        stream << tab5;
                        switch(m)
                        {
                            case 0 : stream << "if (! par.es)"; break;
                            case 1 : stream << "else if (pj_param(par.params, \"bguam\").i)"; break;
                            case 2 : stream << "else"; break;
                        }
                        stream << std::endl;
                    }
                    else if (projection_group == "gn_sinu" && dit->name != "sinu" && m == 0)
                    {
                        skip = true;
                    }

                    if (! skip)
                    {
                        std::string base = "base_v_f";
                        std::map<std::string, bool>::iterator it = has_inverse.find(*mit);
                        if (it != has_inverse.end() && it->second)
                        {
                            base += "i";
                        }
                        std::string name = dit->name + "_" + *mit;
                        stream << tab << "return new " << base
                            << "<" << name << "<Geographic, Cartesian, Parameters>, Geographic, Cartesian, Parameters>(par);" << std::endl;

                        if (epsg_model.empty())
                        {
                            epsg_model = *mit;
                        }
                    }
                }
                stream
                    << tab4 << "}" << std::endl
                    << tab2 << "};" << std::endl << std::endl;

            }

            // Create "PRJ_init" function for registration at factory
            stream << tab2 << "" << templates << std::endl
                << tab2 << "inline void " << projection_group << "_init(detail::base_factory<Geographic, Cartesian, Parameters>& factory)" << std::endl
                << tab2 << "{" << std::endl;
            for (std::vector<derived>::const_iterator dit = derived_projections.begin(); dit != derived_projections.end(); dit++)
            {
                stream << tab3 << "factory.add_to_factory(\"" << dit->name << "\", new "
                    << dit->name << "_entry<Geographic, Cartesian, Parameters>);" << std::endl;
            }
            stream << tab2 << "}" << std::endl << std::endl;

            stream << tab1 << "} // namespace detail " << std::endl;


            // Create EPSG specializations
            if (use_epsg())
            {
                stream << tab1 << "// Create EPSG specializations" << std::endl
                    << tab1 << "// (Proof of Concept, only for some)" << std::endl
                    << std::endl;
                for (std::vector<derived>::const_iterator dit = derived_projections.begin(); dit != derived_projections.end(); dit++)
                {
                    for (std::vector<epsg_entry>::const_iterator eit = epsg_entries.begin(); eit != epsg_entries.end(); eit++)
                    {
                        if (eit->prj_name == dit->name)
                        {
                            stream << tab1 << "template<typename LatLongRadian, typename Cartesian, typename Parameters>" << std::endl
                                << tab1 << "struct epsg_traits<" << eit->epsg_code << ", LatLongRadian, Cartesian, Parameters>" << std::endl
                                << tab1 << "{" << std::endl
                                // TODO, model, see above
                                << tab2 << "typedef " << dit->name << "_" << epsg_model << "<LatLongRadian, Cartesian, Parameters> type;" << std::endl
                                << tab2 << "static inline std::string par()" << std::endl
                                << tab2 << "{" << std::endl
                                << tab3 << "return \"" << eit->parameters << "\";" << std::endl
                                << tab2 << "}" << std::endl
                                << tab1 << "};" << std::endl
                                << std::endl
                                << std::endl;
                        }
                    }

                }
            }

            stream
                << tab1 << "#endif // doxygen" << std::endl
                << std::endl;


        };


        void write()
        {
            stream << "#ifndef " << hpp << std::endl
                << "#define " << hpp << std::endl
                << std::endl;

            write_copyright();
            write_header();
            write_begin_impl();
            {
                write_consts();
                write_proj_par_struct();
                write_prefix();
                write_impl_classes();
                write_postfix();
                write_setup();
            }
            write_end_impl();

            write_classes();

            write_wrappers();
            write_end();
        }


        std::string projection_group;
        std::string hpp;

        std::vector<projection> projections;
        std::vector<derived> derived_projections;

        std::vector<define> defined_consts;
        std::vector<define> defined_parameters;

        // contain functions for use in projections / common code for constructors etc
        std::vector<std::string> prefix; // can contain functions
        std::vector<std::string> postfix; // can contain setup function(s)
        std::vector<std::string> proj_parameters; // will be written as struct
        std::vector<std::string> proj_parameter_names; // same but only names
        std::string setup_function_line;



        std::vector<std::string> models; // Filled during write_classes
        std::map<std::string, bool> has_inverse; // Per model, also filled

        bool skip_for_setup;
        unsigned int skip_lineno;
        unsigned int lineno;
};



int main (int argc, char** argv)
{
    fill_epsg_entries();

    if (argc < 3)
    {
        std::cerr << "USAGE: " << argv[0] << " <source file> <group name>" << std::endl;
        return 1;
    }

    try
    {
        proj4_converter(argv[1], argv[2], std::cout);
    }
    catch(std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
    }


    return 0;
}
