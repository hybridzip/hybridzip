#include "dotenv.h"

#include "environ.h"
#include "Parser.h"

#include <fstream>
#include <utility>


using namespace std;
using namespace cpp_dotenv;


cpp_dotenv::dotenv& cpp_dotenv::dotenv::load_dotenv(const string& dotenv_path, const bool overwrite, const bool interpolate)
{
    ifstream env_file;
    env_file.open(dotenv_path);

    if (env_file.good())
    {
        Parser parser;
        parser.parse(env_file, overwrite, interpolate);
        env_file.close();
    }

    return *this;
}


const cpp_dotenv::dotenv::value_type cpp_dotenv::dotenv::operator[](const key_type& k) const
{
    return getenv(k).second;
}


cpp_dotenv::dotenv& cpp_dotenv::dotenv::instance()
{
    return _instance;
}


const string cpp_dotenv::dotenv::env_filename = ".env";
cpp_dotenv::dotenv cpp_dotenv::dotenv::_instance;

cpp_dotenv::dotenv& cpp_dotenv::env = dotenv::instance();
