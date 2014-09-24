#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <vector>
#include <tuple>

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::ofstream;
using std::vector;
using std::tuple;
using std::tie;
using std::make_tuple;

#include "io.h"
#include "matrix.h"
#include "ImageProcessor.h"

typedef tuple<uint, uint, uint, uint> Rect;

tuple<vector<Rect>, Image>
find_treasure(const Image& in)
{
    // Base: return Rect of treasure only
    // Bonus: return Rects of arrows and then treasure Rect
    auto path = vector<Rect>();
    return make_tuple(path, in.deep_copy());
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        cout << argv[1] << endl;
        cout << argv[2] << endl;
        cout << argv[3] << endl;
        cout << "Usage: " << endl << argv[0]
             << " <in_image.bmp> <out_image.bmp> <out_path.txt>" << endl;
        return 0;
    }

    try {
        std::shared_ptr<Image> src_image = std::make_shared<Image>(load_image(argv[1]));
        ImageProcessor imProc = ImageProcessor(src_image);
        imProc.whitenPixels();
        save_image(*src_image, argv[2]);
        /*ofstream fout(argv[3]);

        vector<Rect> path;
        Image dst_image;
        tie(path, dst_image) = find_treasure(src_image);
        save_image(dst_image, argv[2]);

        uint x, y, width, height;
        for (const auto &obj : path)
        {
            tie(x, y, width, height) = obj;
            fout << x << " " << y << " " << width << " " << height << endl;
        }
        */
    }
        catch (const string &s) {
        cerr << "Error: " << s << endl;
        return 1;
    }
}
