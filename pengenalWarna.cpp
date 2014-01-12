#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"

using namespace std;
using namespace cv;

#define MIN3(x,y,z)  ((y) <= (z) ? \
                         ((x) <= (y) ? (x) : (y)) \
                     : \
                         ((x) <= (z) ? (x) : (z)))

#define MAX3(x,y,z)  ((y) >= (z) ? \
                         ((x) >= (y) ? (x) : (y)) \
                     : \
                         ((x) >= (z) ? (x) : (z)))

struct RGB 
{
    unsigned __int8 r;		/* Red			[0-255] */
	unsigned __int8 g;		/* Green		[0-255] */
	unsigned __int8 b;		/* Ble			[0-255] */
};

struct HSL 
{
    unsigned __int8 h;	/* Hue			[0-180] */
    unsigned __int8 s;	/* Saturation	[0-255] (gray)*/
    unsigned __int8 l;	/* Lightness	[0-255] (black)*/
};

/***************CLASS WARNA***************/
class Warna
{

public:
	void setNama(string nama){ this->nama = nama; }
	string getNama(){ return this->nama; }
	void setRgb(unsigned __int8 r, unsigned __int8 g, unsigned __int8 b);
	struct RGB getRgb(){ return this->rgb; }
	void setHsl(unsigned __int8 h, unsigned __int8 s, unsigned __int8 l);
	struct HSL getHsl(){ return this->hsl; }

private:
	string nama;
	struct RGB rgb;
	struct HSL hsl;
	void rgb2hsl();
};

void Warna::rgb2hsl()
{
	double	rgb_min, rgb_max, delta, 
			h, s, l,
			r, g, b;
	
	
	r = (double)this->rgb.r /255;
	g = (double)this->rgb.g /255;
	b = (double)this->rgb.b /255;

	rgb_min = MIN3(r, g, b);
    rgb_max = MAX3(r, g, b);

	delta = rgb_max - rgb_min;

	//hitung lightness//
	l = (rgb_max + rgb_min)/2;
    
	if (rgb_max == 0) {
		
		this->hsl.h = this->hsl.s = this->hsl.l = 0;
    }else{

		//hitung saturation//
		if(delta == 0){
			s = 0;
		}else{
			s = delta/(1 - abs(2 * l - 1));
		}
	
		/*hitung hue*/
		if (rgb_max == r) {
			h = 60 *  fmod(((g - b)/delta), 6);
			if (h < 0.0) {
				h += 360;
			}
		} else if (rgb_max == g) {
			h = 60 * ( ((b - r)/delta) + 2);
		} else { /* rgb_max == b */
			h = 60 * ( ((r - g)/delta) + 4);
		}

		this->hsl.h = (unsigned __int8)(h / 2);
		this->hsl.s	= (unsigned __int8)(255 * s);
		this->hsl.l = (unsigned __int8)(255 * l);
	}
}

void Warna::setHsl(unsigned __int8 h, unsigned __int8 s, unsigned __int8 l)
{
	this->hsl.h = h; 
	this->hsl.s = s; 
	this->hsl.l = l;
}

void Warna::setRgb(unsigned __int8 r, unsigned __int8 g, unsigned __int8 b)
{ 
	this->rgb.r = r; 
	this->rgb.g = g; 
	this->rgb.b = b;

	this->rgb2hsl();
}

/***************CLASS Control Warna***************/
class ControlWarna
{
public :
	ControlWarna();
	Warna cariWarnaIdentik(Warna warnaInput);
private :
	vector<Warna> daftarWarna;
	void inisialisasiDataWarna();
};

Warna ControlWarna::cariWarnaIdentik(Warna warnaInput)
{
	int cek = -1, ndrgb, ndhsl, ncek = 0, indwar;

	int inx = 0;
	for(Warna &i: this->daftarWarna){
		ndrgb = abs(warnaInput.getRgb().r - i.getRgb().r) + abs(warnaInput.getRgb().g - i.getRgb().g) + abs(warnaInput.getRgb().b - i.getRgb().b);
		ndhsl = abs(warnaInput.getHsl().h - i.getHsl().h) + abs(warnaInput.getHsl().s - i.getHsl().s) + abs(warnaInput.getHsl().l - i.getHsl().l);
		ncek = ndrgb + ndhsl;
		
		if(cek < 0 || cek > ncek){
			cek = ncek;
			indwar = inx;
		}
		inx++;
	}

	return this->daftarWarna[indwar];
}

ControlWarna::ControlWarna()
{
	this->inisialisasiDataWarna();
}

/***************CLASS LIVE CAM***************/
class LiveCam
{
public:
	LiveCam();
	Mat frame;
	VideoCapture capture;
	bool getFrame();
	Warna outputWarna;
	bool mulai;
	ControlWarna controlWarna;	
};



bool LiveCam::getFrame()
{
	return capture.read(frame);
}

LiveCam::LiveCam()
{
	mulai = false;

	this->capture.open(0);

	if(!this->capture.isOpened())
	{
		cout<<"Gagal membuka perangkat kamera"<<endl;
	}
	assert(this->capture.isOpened());
}

/***************MAIN PROGRAM***************/

LiveCam lc;

void pilihObject(int event, int x, int y, int flags, void* userdata)
{
	if  ( event == EVENT_LBUTTONDOWN ){
		Warna warnaInput;
		warnaInput.setRgb(lc.frame.at<Vec3b>(y,x)[2], lc.frame.at<Vec3b>(y,x)[1], lc.frame.at<Vec3b>(y,x)[0]);

		lc.outputWarna = lc.controlWarna.cariWarnaIdentik(warnaInput);
		cout<<"warna yang identik adalah "<<lc.outputWarna.getNama()<<endl;
		

		lc.mulai = true;
	}
}

int main(int argc, char *argv[])
{
	

	namedWindow("cam_0", WINDOW_NORMAL);
	setMouseCallback("cam_0", pilihObject);

	while (true)
	{
		if(lc.getFrame()){

			if(lc.mulai){
				//tampilkan warna identik yang di click
				Mat_<Vec3b> _I = lc.frame;
				for( int i = 0; i < lc.frame.rows / 8; ++i)
					for( int j = 0; j < lc.frame.cols / 8; ++j ){
						_I(i,j)[0] = lc.outputWarna.getRgb().b;
						_I(i,j)[1] = lc.outputWarna.getRgb().g;
						_I(i,j)[2] = lc.outputWarna.getRgb().r;
					}
		
				lc.frame = _I;  
			}

			imshow("cam_0", lc.frame);
		}
		if(waitKey(30) == 27) break;
	}
}

/***************inisialisasi data warna***************/

void ControlWarna::inisialisasiDataWarna()
{
	Warna warna;

	warna.setNama("Air Force blue");
	warna.setRgb(93, 138, 168 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Alice blue");
	warna.setRgb(240, 248, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Alizarin crimson");
	warna.setRgb(227, 38, 54 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Almond");
	warna.setRgb(239, 222, 205 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Amaranth");
	warna.setRgb(229, 43, 80 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Amber");
	warna.setRgb(255, 191, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("American rose");
	warna.setRgb(255, 3, 62 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Amethyst");
	warna.setRgb(153, 102, 204 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Android Green");
	warna.setRgb(164, 198, 57 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Anti-flash white");
	warna.setRgb(242, 243, 244 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Antique brass");
	warna.setRgb(205, 149, 117 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Antique fuchsia");
	warna.setRgb(145, 92, 131 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Antique white");
	warna.setRgb(250, 235, 215 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ao");
	warna.setRgb(0, 128, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Apple green");
	warna.setRgb(141, 182, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Apricot");
	warna.setRgb(251, 206, 177 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Aqua");
	warna.setRgb(0, 255, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Aquamarine");
	warna.setRgb(127, 255, 212 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Army green");
	warna.setRgb(75, 83, 32 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Arylide yellow");
	warna.setRgb(233, 214, 107 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ash grey");
	warna.setRgb(178, 190, 181 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Asparagus");
	warna.setRgb(135, 169, 107 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Atomic tangerine");
	warna.setRgb(255, 153, 102 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Auburn");
	warna.setRgb(165, 42, 42 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Aureolin");
	warna.setRgb(253, 238, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("AuroMetalSaurus");
	warna.setRgb(110, 127, 128 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Awesome");
	warna.setRgb(255, 32, 82 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Azure");
	warna.setRgb(0, 127, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Azure mist/web");
	warna.setRgb(240, 255, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Baby blue");
	warna.setRgb(137, 207, 240 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Baby blue eyes");
	warna.setRgb(161, 202, 241 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Baby pink");
	warna.setRgb(244, 194, 194 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ball Blue");
	warna.setRgb(33, 171, 205 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Banana Mania");
	warna.setRgb(250, 231, 181 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Banana yellow");
	warna.setRgb(255, 225, 53 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Battleship grey");
	warna.setRgb(132, 132, 130 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bazaar");
	warna.setRgb(152, 119, 123 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Beau blue");
	warna.setRgb(188, 212, 230 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Beaver");
	warna.setRgb(159, 129, 112 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Beige");
	warna.setRgb(245, 245, 220 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bisque");
	warna.setRgb(255, 228, 196 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bistre");
	warna.setRgb(61, 43, 31 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bittersweet");
	warna.setRgb(254, 111, 94 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Black");
	warna.setRgb(0, 0, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Blanched Almond");
	warna.setRgb(255, 235, 205 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bleu de France");
	warna.setRgb(49, 140, 231 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Blizzard Blue");
	warna.setRgb(172, 229, 238 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Blond");
	warna.setRgb(250, 240, 190 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Blue");
	warna.setRgb(0, 0, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Blue Bell");
	warna.setRgb(162, 162, 208 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Blue Gray");
	warna.setRgb(102, 153, 204 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Blue green");
	warna.setRgb(13, 152, 186 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Blue purple");
	warna.setRgb(138, 43, 226 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Blue violet");
	warna.setRgb(138, 43, 226 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Blush");
	warna.setRgb(222, 93, 131 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bole");
	warna.setRgb(121, 68, 59 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bondi blue");
	warna.setRgb(0, 149, 182 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bone");
	warna.setRgb(227, 218, 201 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Boston University Red");
	warna.setRgb(204, 0, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bottle green");
	warna.setRgb(0, 106, 78 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Boysenberry");
	warna.setRgb(135, 50, 96 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Brandeis blue");
	warna.setRgb(0, 112, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Brass");
	warna.setRgb(181, 166, 66 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Brick red");
	warna.setRgb(203, 65, 84 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bright cerulean");
	warna.setRgb(29, 172, 214 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bright green");
	warna.setRgb(102, 255, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bright lavender");
	warna.setRgb(191, 148, 228 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bright maroon");
	warna.setRgb(195, 33, 72 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bright pink");
	warna.setRgb(255, 0, 127 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bright turquoise");
	warna.setRgb(8, 232, 222 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bright ube");
	warna.setRgb(209, 159, 232 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Brilliant lavender");
	warna.setRgb(244, 187, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Brilliant rose");
	warna.setRgb(255, 85, 163 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Brink pink");
	warna.setRgb(251, 96, 127 );
	this->daftarWarna.push_back(warna);

	warna.setNama("British racing green");
	warna.setRgb(0, 66, 37 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bronze");
	warna.setRgb(205, 127, 50 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Brown");
	warna.setRgb(165, 42, 42 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bubble gum");
	warna.setRgb(255, 193, 204 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bubbles");
	warna.setRgb(231, 254, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Buff");
	warna.setRgb(240, 220, 130 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Bulgarian rose");
	warna.setRgb(72, 6, 7 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Burgundy");
	warna.setRgb(128, 0, 32 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Burlywood");
	warna.setRgb(222, 184, 135 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Burnt orange");
	warna.setRgb(204, 85, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Burnt sienna");
	warna.setRgb(233, 116, 81 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Burnt umber");
	warna.setRgb(138, 51, 36 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Byzantine");
	warna.setRgb(189, 51, 164 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Byzantium");
	warna.setRgb(112, 41, 99 );
	this->daftarWarna.push_back(warna);

	warna.setNama("CG Blue");
	warna.setRgb(0, 122, 165 );
	this->daftarWarna.push_back(warna);

	warna.setNama("CG Red");
	warna.setRgb(224, 60, 49 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cadet");
	warna.setRgb(83, 104, 114 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cadet blue");
	warna.setRgb(95, 158, 160 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cadet grey");
	warna.setRgb(145, 163, 176 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cadmium green");
	warna.setRgb(0, 107, 60 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cadmium orange");
	warna.setRgb(237, 135, 45 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cadmium red");
	warna.setRgb(227, 0, 34 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cadmium yellow");
	warna.setRgb(255, 246, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Café au lait");
	warna.setRgb(166, 123, 91 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Café noir");
	warna.setRgb(75, 54, 33 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cal Poly Pomona green");
	warna.setRgb(30, 77, 43 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cambridge Blue");
	warna.setRgb(163, 193, 173 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Camel");
	warna.setRgb(193, 154, 107 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Camouflage green");
	warna.setRgb(120, 134, 107 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Canary");
	warna.setRgb(255, 255, 153 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Canary yellow");
	warna.setRgb(255, 239, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Candy apple red");
	warna.setRgb(255, 8, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Candy pink");
	warna.setRgb(228, 113, 122 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Capri");
	warna.setRgb(0, 191, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Caput mortuum");
	warna.setRgb(89, 39, 32 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cardinal");
	warna.setRgb(196, 30, 58 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Caribbean green");
	warna.setRgb(0, 204, 153 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Carmine");
	warna.setRgb(255, 0, 64 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Carmine pink");
	warna.setRgb(235, 76, 66 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Carmine red");
	warna.setRgb(255, 0, 56 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Carnation pink");
	warna.setRgb(255, 166, 201 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Carnelian");
	warna.setRgb(179, 27, 27 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Carolina blue");
	warna.setRgb(153, 186, 221 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Carrot orange");
	warna.setRgb(237, 145, 33 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Celadon");
	warna.setRgb(172, 225, 175 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Celeste");
	warna.setRgb(178, 255, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Celestial blue");
	warna.setRgb(73, 151, 208 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cerise");
	warna.setRgb(222, 49, 99 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cerise pink");
	warna.setRgb(236, 59, 131 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cerulean");
	warna.setRgb(0, 123, 167 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cerulean blue");
	warna.setRgb(42, 82, 190 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Chamoisee");
	warna.setRgb(160, 120, 90 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Champagne");
	warna.setRgb(250, 214, 165 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Charcoal");
	warna.setRgb(54, 69, 79 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Chartreuse");
	warna.setRgb(127, 255, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cherry");
	warna.setRgb(222, 49, 99 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cherry blossom pink");
	warna.setRgb(255, 183, 197 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Chestnut");
	warna.setRgb(205, 92, 92 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Chocolate");
	warna.setRgb(210, 105, 30 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Chrome yellow");
	warna.setRgb(255, 167, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cinereous");
	warna.setRgb(152, 129, 123 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cinnabar");
	warna.setRgb(227, 66, 52 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cinnamon");
	warna.setRgb(210, 105, 30 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Citrine");
	warna.setRgb(228, 208, 10 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Classic rose");
	warna.setRgb(251, 204, 231 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cobalt");
	warna.setRgb(0, 71, 171 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cocoa brown");
	warna.setRgb(210, 105, 30 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Coffee");
	warna.setRgb(111, 78, 55 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Columbia blue");
	warna.setRgb(155, 221, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cool black");
	warna.setRgb(0, 46, 99 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cool grey");
	warna.setRgb(140, 146, 172 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Copper");
	warna.setRgb(184, 115, 51 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Copper rose");
	warna.setRgb(153, 102, 102 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Coquelicot");
	warna.setRgb(255, 56, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Coral");
	warna.setRgb(255, 127, 80 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Coral pink");
	warna.setRgb(248, 131, 121 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Coral red");
	warna.setRgb(255, 64, 64 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cordovan");
	warna.setRgb(137, 63, 69 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Corn");
	warna.setRgb(251, 236, 93 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cornell Red");
	warna.setRgb(179, 27, 27 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cornflower");
	warna.setRgb(154, 206, 235 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cornflower blue");
	warna.setRgb(100, 149, 237 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cornsilk");
	warna.setRgb(255, 248, 220 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cosmic latte");
	warna.setRgb(255, 248, 231 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cotton candy");
	warna.setRgb(255, 188, 217 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cream");
	warna.setRgb(255, 253, 208 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Crimson");
	warna.setRgb(220, 20, 60 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Crimson Red");
	warna.setRgb(153, 0, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Crimson glory");
	warna.setRgb(190, 0, 50 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Cyan");
	warna.setRgb(0, 255, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Daffodil");
	warna.setRgb(255, 255, 49 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dandelion");
	warna.setRgb(240, 225, 48 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark blue");
	warna.setRgb(0, 0, 139 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark brown");
	warna.setRgb(101, 67, 33 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark byzantium");
	warna.setRgb(93, 57, 84 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark candy apple red");
	warna.setRgb(164, 0, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark cerulean");
	warna.setRgb(8, 69, 126 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark chestnut");
	warna.setRgb(152, 105, 96 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark coral");
	warna.setRgb(205, 91, 69 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark cyan");
	warna.setRgb(0, 139, 139 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark electric blue");
	warna.setRgb(83, 104, 120 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark goldenrod");
	warna.setRgb(184, 134, 11 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark gray");
	warna.setRgb(169, 169, 169 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark green");
	warna.setRgb(1, 50, 32 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark jungle green");
	warna.setRgb(26, 36, 33 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark khaki");
	warna.setRgb(189, 183, 107 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark lava");
	warna.setRgb(72, 60, 50 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark lavender");
	warna.setRgb(115, 79, 150 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark magenta");
	warna.setRgb(139, 0, 139 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark midnight blue");
	warna.setRgb(0, 51, 102 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark olive green");
	warna.setRgb(85, 107, 47 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark orange");
	warna.setRgb(255, 140, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark orchid");
	warna.setRgb(153, 50, 204 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark pastel blue");
	warna.setRgb(119, 158, 203 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark pastel green");
	warna.setRgb(3, 192, 60 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark pastel purple");
	warna.setRgb(150, 111, 214 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark pastel red");
	warna.setRgb(194, 59, 34 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark pink");
	warna.setRgb(231, 84, 128 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark powder blue");
	warna.setRgb(0, 51, 153 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark raspberry");
	warna.setRgb(135, 38, 87 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark red");
	warna.setRgb(139, 0, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark salmon");
	warna.setRgb(233, 150, 122 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark scarlet");
	warna.setRgb(86, 3, 25 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark sea green");
	warna.setRgb(143, 188, 143 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark sienna");
	warna.setRgb(60, 20, 20 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark slate blue");
	warna.setRgb(72, 61, 139 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark slate gray");
	warna.setRgb(47, 79, 79 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark spring green");
	warna.setRgb(23, 114, 69 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark tan");
	warna.setRgb(145, 129, 81 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark tangerine");
	warna.setRgb(255, 168, 18 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark taupe");
	warna.setRgb(72, 60, 50 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark terra cotta");
	warna.setRgb(204, 78, 92 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark turquoise");
	warna.setRgb(0, 206, 209 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dark violet");
	warna.setRgb(148, 0, 211 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dartmouth green");
	warna.setRgb(0, 105, 62 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Davy grey");
	warna.setRgb(85, 85, 85 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Debian red");
	warna.setRgb(215, 10, 83 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Deep carmine");
	warna.setRgb(169, 32, 62 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Deep carmine pink");
	warna.setRgb(239, 48, 56 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Deep carrot orange");
	warna.setRgb(233, 105, 44 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Deep cerise");
	warna.setRgb(218, 50, 135 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Deep champagne");
	warna.setRgb(250, 214, 165 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Deep chestnut");
	warna.setRgb(185, 78, 72 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Deep coffee");
	warna.setRgb(112, 66, 65 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Deep fuchsia");
	warna.setRgb(193, 84, 193 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Deep jungle green");
	warna.setRgb(0, 75, 73 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Deep lilac");
	warna.setRgb(153, 85, 187 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Deep magenta");
	warna.setRgb(204, 0, 204 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Deep peach");
	warna.setRgb(255, 203, 164 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Deep pink");
	warna.setRgb(255, 20, 147 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Deep saffron");
	warna.setRgb(255, 153, 51 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Deep sky blue");
	warna.setRgb(0, 191, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Denim");
	warna.setRgb(21, 96, 189 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Desert");
	warna.setRgb(193, 154, 107 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Desert sand");
	warna.setRgb(237, 201, 175 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dim gray");
	warna.setRgb(105, 105, 105 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dodger blue");
	warna.setRgb(30, 144, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dogwood rose");
	warna.setRgb(215, 24, 104 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Dollar bill");
	warna.setRgb(133, 187, 101 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Drab");
	warna.setRgb(150, 113, 23 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Duke blue");
	warna.setRgb(0, 0, 156 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Earth yellow");
	warna.setRgb(225, 169, 95 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ecru");
	warna.setRgb(194, 178, 128 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Eggplant");
	warna.setRgb(97, 64, 81 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Eggshell");
	warna.setRgb(240, 234, 214 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Egyptian blue");
	warna.setRgb(16, 52, 166 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Electric blue");
	warna.setRgb(125, 249, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Electric crimson");
	warna.setRgb(255, 0, 63 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Electric cyan");
	warna.setRgb(0, 255, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Electric green");
	warna.setRgb(0, 255, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Electric indigo");
	warna.setRgb(111, 0, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Electric lavender");
	warna.setRgb(244, 187, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Electric lime");
	warna.setRgb(204, 255, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Electric purple");
	warna.setRgb(191, 0, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Electric ultramarine");
	warna.setRgb(63, 0, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Electric violet");
	warna.setRgb(143, 0, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Electric yellow");
	warna.setRgb(255, 255, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Emerald");
	warna.setRgb(80, 200, 120 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Eton blue");
	warna.setRgb(150, 200, 162 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Fallow");
	warna.setRgb(193, 154, 107 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Falu red");
	warna.setRgb(128, 24, 24 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Famous");
	warna.setRgb(255, 0, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Fandango");
	warna.setRgb(181, 51, 137 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Fashion fuchsia");
	warna.setRgb(244, 0, 161 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Fawn");
	warna.setRgb(229, 170, 112 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Feldgrau");
	warna.setRgb(77, 93, 83 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Fern");
	warna.setRgb(113, 188, 120 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Fern green");
	warna.setRgb(79, 121, 66 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ferrari Red");
	warna.setRgb(255, 40, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Field drab");
	warna.setRgb(108, 84, 30 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Fire engine red");
	warna.setRgb(206, 32, 41 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Firebrick");
	warna.setRgb(178, 34, 34 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Flame");
	warna.setRgb(226, 88, 34 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Flamingo pink");
	warna.setRgb(252, 142, 172 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Flavescent");
	warna.setRgb(247, 233, 142 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Flax");
	warna.setRgb(238, 220, 130 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Floral white");
	warna.setRgb(255, 250, 240 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Fluorescent orange");
	warna.setRgb(255, 191, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Fluorescent pink");
	warna.setRgb(255, 20, 147 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Fluorescent yellow");
	warna.setRgb(204, 255, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Folly");
	warna.setRgb(255, 0, 79 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Forest green");
	warna.setRgb(34, 139, 34 );
	this->daftarWarna.push_back(warna);

	warna.setNama("French beige");
	warna.setRgb(166, 123, 91 );
	this->daftarWarna.push_back(warna);

	warna.setNama("French blue");
	warna.setRgb(0, 114, 187 );
	this->daftarWarna.push_back(warna);

	warna.setNama("French lilac");
	warna.setRgb(134, 96, 142 );
	this->daftarWarna.push_back(warna);

	warna.setNama("French rose");
	warna.setRgb(246, 74, 138 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Fuchsia");
	warna.setRgb(255, 0, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Fuchsia pink");
	warna.setRgb(255, 119, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Fulvous");
	warna.setRgb(228, 132, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Fuzzy Wuzzy");
	warna.setRgb(204, 102, 102 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Gainsboro");
	warna.setRgb(220, 220, 220 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Gamboge");
	warna.setRgb(228, 155, 15 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ghost white");
	warna.setRgb(248, 248, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ginger");
	warna.setRgb(176, 101, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Glaucous");
	warna.setRgb(96, 130, 182 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Glitter");
	warna.setRgb(230, 232, 250 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Gold");
	warna.setRgb(255, 215, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Golden brown");
	warna.setRgb(153, 101, 21 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Golden poppy");
	warna.setRgb(252, 194, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Golden yellow");
	warna.setRgb(255, 223, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Goldenrod");
	warna.setRgb(218, 165, 32 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Granny Smith Apple");
	warna.setRgb(168, 228, 160 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Gray");
	warna.setRgb(128, 128, 128 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Gray asparagus");
	warna.setRgb(70, 89, 69 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Green");
	warna.setRgb(0, 255, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Green Blue");
	warna.setRgb(17, 100, 180 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Green yellow");
	warna.setRgb(173, 255, 47 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Grullo");
	warna.setRgb(169, 154, 134 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Guppie green");
	warna.setRgb(0, 255, 127 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Halay�  úbe");
	warna.setRgb(102, 56, 84 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Han blue");
	warna.setRgb(68, 108, 207 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Han purple");
	warna.setRgb(82, 24, 250 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Hansa yellow");
	warna.setRgb(233, 214, 107 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Harlequin");
	warna.setRgb(63, 255, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Harvard crimson");
	warna.setRgb(201, 0, 22 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Harvest Gold");
	warna.setRgb(218, 145, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Heart Gold");
	warna.setRgb(128, 128, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Heliotrope");
	warna.setRgb(223, 115, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Hollywood cerise");
	warna.setRgb(244, 0, 161 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Honeydew");
	warna.setRgb(240, 255, 240 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Hooker green");
	warna.setRgb(73, 121, 107 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Hot magenta");
	warna.setRgb(255, 29, 206 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Hot pink");
	warna.setRgb(255, 105, 180 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Hunter green");
	warna.setRgb(53, 94, 59 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Icterine");
	warna.setRgb(252, 247, 94 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Inchworm");
	warna.setRgb(178, 236, 93 );
	this->daftarWarna.push_back(warna);

	warna.setNama("India green");
	warna.setRgb(19, 136, 8 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Indian red");
	warna.setRgb(205, 92, 92 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Indian yellow");
	warna.setRgb(227, 168, 87 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Indigo");
	warna.setRgb(75, 0, 130 );
	this->daftarWarna.push_back(warna);

	warna.setNama("International Klein Blue");
	warna.setRgb(0, 47, 167 );
	this->daftarWarna.push_back(warna);

	warna.setNama("International orange");
	warna.setRgb(255, 79, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Iris");
	warna.setRgb(90, 79, 207 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Isabelline");
	warna.setRgb(244, 240, 236 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Islamic green");
	warna.setRgb(0, 144, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ivory");
	warna.setRgb(255, 255, 240 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Jade");
	warna.setRgb(0, 168, 107 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Jasmine");
	warna.setRgb(248, 222, 126 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Jasper");
	warna.setRgb(215, 59, 62 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Jazzberry jam");
	warna.setRgb(165, 11, 94 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Jonquil");
	warna.setRgb(250, 218, 94 );
	this->daftarWarna.push_back(warna);

	warna.setNama("June bud");
	warna.setRgb(189, 218, 87 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Jungle green");
	warna.setRgb(41, 171, 135 );
	this->daftarWarna.push_back(warna);

	warna.setNama("KU Crimson");
	warna.setRgb(232, 0, 13 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Kelly green");
	warna.setRgb(76, 187, 23 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Khaki");
	warna.setRgb(195, 176, 145 );
	this->daftarWarna.push_back(warna);

	warna.setNama("La Salle Green");
	warna.setRgb(8, 120, 48 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Languid lavender");
	warna.setRgb(214, 202, 221 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lapis lazuli");
	warna.setRgb(38, 97, 156 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Laser Lemon");
	warna.setRgb(254, 254, 34 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Laurel green");
	warna.setRgb(169, 186, 157 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lava");
	warna.setRgb(207, 16, 32 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lavender");
	warna.setRgb(230, 230, 250 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lavender blue");
	warna.setRgb(204, 204, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lavender blush");
	warna.setRgb(255, 240, 245 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lavender gray");
	warna.setRgb(196, 195, 208 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lavender indigo");
	warna.setRgb(148, 87, 235 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lavender magenta");
	warna.setRgb(238, 130, 238 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lavender mist");
	warna.setRgb(230, 230, 250 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lavender pink");
	warna.setRgb(251, 174, 210 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lavender purple");
	warna.setRgb(150, 123, 182 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lavender rose");
	warna.setRgb(251, 160, 227 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lawn green");
	warna.setRgb(124, 252, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lemon");
	warna.setRgb(255, 247, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lemon Yellow");
	warna.setRgb(255, 244, 79 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lemon chiffon");
	warna.setRgb(255, 250, 205 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lemon lime");
	warna.setRgb(191, 255, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light Crimson");
	warna.setRgb(245, 105, 145 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light Thulian pink");
	warna.setRgb(230, 143, 172 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light apricot");
	warna.setRgb(253, 213, 177 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light blue");
	warna.setRgb(173, 216, 230 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light brown");
	warna.setRgb(181, 101, 29 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light carmine pink");
	warna.setRgb(230, 103, 113 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light coral");
	warna.setRgb(240, 128, 128 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light cornflower blue");
	warna.setRgb(147, 204, 234 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light cyan");
	warna.setRgb(224, 255, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light fuchsia pink");
	warna.setRgb(249, 132, 239 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light goldenrod yellow");
	warna.setRgb(250, 250, 210 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light gray");
	warna.setRgb(211, 211, 211 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light green");
	warna.setRgb(144, 238, 144 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light khaki");
	warna.setRgb(240, 230, 140 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light pastel purple");
	warna.setRgb(177, 156, 217 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light pink");
	warna.setRgb(255, 182, 193 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light salmon");
	warna.setRgb(255, 160, 122 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light salmon pink");
	warna.setRgb(255, 153, 153 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light sea green");
	warna.setRgb(32, 178, 170 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light sky blue");
	warna.setRgb(135, 206, 250 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light slate gray");
	warna.setRgb(119, 136, 153 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light taupe");
	warna.setRgb(179, 139, 109 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Light yellow");
	warna.setRgb(255, 255, 237 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lilac");
	warna.setRgb(200, 162, 200 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lime");
	warna.setRgb(191, 255, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lime green");
	warna.setRgb(50, 205, 50 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lincoln green");
	warna.setRgb(25, 89, 5 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Linen");
	warna.setRgb(250, 240, 230 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lion");
	warna.setRgb(193, 154, 107 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Liver");
	warna.setRgb(83, 75, 79 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Lust");
	warna.setRgb(230, 32, 32 );
	this->daftarWarna.push_back(warna);

	warna.setNama("MSU Green");
	warna.setRgb(24, 69, 59 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Macaroni and Cheese");
	warna.setRgb(255, 189, 136 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Magenta");
	warna.setRgb(255, 0, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Magic mint");
	warna.setRgb(170, 240, 209 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Magnolia");
	warna.setRgb(248, 244, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mahogany");
	warna.setRgb(192, 64, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Maize");
	warna.setRgb(251, 236, 93 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Majorelle Blue");
	warna.setRgb(96, 80, 220 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Malachite");
	warna.setRgb(11, 218, 81 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Manatee");
	warna.setRgb(151, 154, 170 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mango Tango");
	warna.setRgb(255, 130, 67 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mantis");
	warna.setRgb(116, 195, 101 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Maroon");
	warna.setRgb(128, 0, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mauve");
	warna.setRgb(224, 176, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mauve taupe");
	warna.setRgb(145, 95, 109 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mauvelous");
	warna.setRgb(239, 152, 170 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Maya blue");
	warna.setRgb(115, 194, 251 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Meat brown");
	warna.setRgb(229, 183, 59 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium Persian blue");
	warna.setRgb(0, 103, 165 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium aquamarine");
	warna.setRgb(102, 221, 170 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium blue");
	warna.setRgb(0, 0, 205 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium candy apple red");
	warna.setRgb(226, 6, 44 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium carmine");
	warna.setRgb(175, 64, 53 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium champagne");
	warna.setRgb(243, 229, 171 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium electric blue");
	warna.setRgb(3, 80, 150 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium jungle green");
	warna.setRgb(28, 53, 45 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium lavender magenta");
	warna.setRgb(221, 160, 221 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium orchid");
	warna.setRgb(186, 85, 211 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium purple");
	warna.setRgb(147, 112, 219 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium red violet");
	warna.setRgb(187, 51, 133 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium sea green");
	warna.setRgb(60, 179, 113 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium slate blue");
	warna.setRgb(123, 104, 238 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium spring bud");
	warna.setRgb(201, 220, 135 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium spring green");
	warna.setRgb(0, 250, 154 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium taupe");
	warna.setRgb(103, 76, 71 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium teal blue");
	warna.setRgb(0, 84, 180 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium turquoise");
	warna.setRgb(72, 209, 204 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Medium violet red");
	warna.setRgb(199, 21, 133 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Melon");
	warna.setRgb(253, 188, 180 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Midnight blue");
	warna.setRgb(25, 25, 112 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Midnight green");
	warna.setRgb(0, 73, 83 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mikado yellow");
	warna.setRgb(255, 196, 12 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mint");
	warna.setRgb(62, 180, 137 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mint cream");
	warna.setRgb(245, 255, 250 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mint green");
	warna.setRgb(152, 255, 152 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Misty rose");
	warna.setRgb(255, 228, 225 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Moccasin");
	warna.setRgb(250, 235, 215 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mode beige");
	warna.setRgb(150, 113, 23 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Moonstone blue");
	warna.setRgb(115, 169, 194 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mordant red 19");
	warna.setRgb(174, 12, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Moss green");
	warna.setRgb(173, 223, 173 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mountain Meadow");
	warna.setRgb(48, 186, 143 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mountbatten pink");
	warna.setRgb(153, 122, 141 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mulberry");
	warna.setRgb(197, 75, 140 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Munsell");
	warna.setRgb(242, 243, 244 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Mustard");
	warna.setRgb(255, 219, 88 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Myrtle");
	warna.setRgb(33, 66, 30 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Nadeshiko pink");
	warna.setRgb(246, 173, 198 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Napier green");
	warna.setRgb(42, 128, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Naples yellow");
	warna.setRgb(250, 218, 94 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Navajo white");
	warna.setRgb(255, 222, 173 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Navy blue");
	warna.setRgb(0, 0, 128 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Neon Carrot");
	warna.setRgb(255, 163, 67 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Neon fuchsia");
	warna.setRgb(254, 89, 194 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Neon green");
	warna.setRgb(57, 255, 20 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Non-photo blue");
	warna.setRgb(164, 221, 237 );
	this->daftarWarna.push_back(warna);

	warna.setNama("North Texas Green");
	warna.setRgb(5, 144, 51 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ocean Boat Blue");
	warna.setRgb(0, 119, 190 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ochre");
	warna.setRgb(204, 119, 34 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Office green");
	warna.setRgb(0, 128, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Old gold");
	warna.setRgb(207, 181, 59 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Old lace");
	warna.setRgb(253, 245, 230 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Old lavender");
	warna.setRgb(121, 104, 120 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Old mauve");
	warna.setRgb(103, 49, 71 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Old rose");
	warna.setRgb(192, 128, 129 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Olive");
	warna.setRgb(128, 128, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Olive Drab");
	warna.setRgb(107, 142, 35 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Olive Green");
	warna.setRgb(186, 184, 108 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Olivine");
	warna.setRgb(154, 185, 115 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Onyx");
	warna.setRgb(15, 15, 15 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Opera mauve");
	warna.setRgb(183, 132, 167 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Orange");
	warna.setRgb(255, 165, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Orange Yellow");
	warna.setRgb(248, 213, 104 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Orange peel");
	warna.setRgb(255, 159, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Orange red");
	warna.setRgb(255, 69, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Orchid");
	warna.setRgb(218, 112, 214 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Otter brown");
	warna.setRgb(101, 67, 33 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Outer Space");
	warna.setRgb(65, 74, 76 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Outrageous Orange");
	warna.setRgb(255, 110, 74 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Oxford Blue");
	warna.setRgb(0, 33, 71 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pacific Blue");
	warna.setRgb(28, 169, 201 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pakistan green");
	warna.setRgb(0, 102, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Palatinate blue");
	warna.setRgb(39, 59, 226 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Palatinate purple");
	warna.setRgb(104, 40, 96 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale aqua");
	warna.setRgb(188, 212, 230 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale blue");
	warna.setRgb(175, 238, 238 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale brown");
	warna.setRgb(152, 118, 84 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale carmine");
	warna.setRgb(175, 64, 53 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale cerulean");
	warna.setRgb(155, 196, 226 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale chestnut");
	warna.setRgb(221, 173, 175 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale copper");
	warna.setRgb(218, 138, 103 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale cornflower blue");
	warna.setRgb(171, 205, 239 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale gold");
	warna.setRgb(230, 190, 138 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale goldenrod");
	warna.setRgb(238, 232, 170 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale green");
	warna.setRgb(152, 251, 152 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale lavender");
	warna.setRgb(220, 208, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale magenta");
	warna.setRgb(249, 132, 229 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale pink");
	warna.setRgb(250, 218, 221 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale plum");
	warna.setRgb(221, 160, 221 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale red violet");
	warna.setRgb(219, 112, 147 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale robin egg blue");
	warna.setRgb(150, 222, 209 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale silver");
	warna.setRgb(201, 192, 187 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale spring bud");
	warna.setRgb(236, 235, 189 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale taupe");
	warna.setRgb(188, 152, 126 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pale violet red");
	warna.setRgb(219, 112, 147 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pansy purple");
	warna.setRgb(120, 24, 74 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Papaya whip");
	warna.setRgb(255, 239, 213 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Paris Green");
	warna.setRgb(80, 200, 120 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pastel blue");
	warna.setRgb(174, 198, 207 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pastel brown");
	warna.setRgb(131, 105, 83 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pastel gray");
	warna.setRgb(207, 207, 196 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pastel green");
	warna.setRgb(119, 221, 119 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pastel magenta");
	warna.setRgb(244, 154, 194 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pastel orange");
	warna.setRgb(255, 179, 71 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pastel pink");
	warna.setRgb(255, 209, 220 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pastel purple");
	warna.setRgb(179, 158, 181 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pastel red");
	warna.setRgb(255, 105, 97 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pastel violet");
	warna.setRgb(203, 153, 201 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pastel yellow");
	warna.setRgb(253, 253, 150 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Patriarch");
	warna.setRgb(128, 0, 128 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Payne grey");
	warna.setRgb(83, 104, 120 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Peach");
	warna.setRgb(255, 229, 180 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Peach puff");
	warna.setRgb(255, 218, 185 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Peach yellow");
	warna.setRgb(250, 223, 173 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pear");
	warna.setRgb(209, 226, 49 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pearl");
	warna.setRgb(234, 224, 200 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pearl Aqua");
	warna.setRgb(136, 216, 192 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Peridot");
	warna.setRgb(230, 226, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Periwinkle");
	warna.setRgb(204, 204, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Persian blue");
	warna.setRgb(28, 57, 187 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Persian indigo");
	warna.setRgb(50, 18, 122 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Persian orange");
	warna.setRgb(217, 144, 88 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Persian pink");
	warna.setRgb(247, 127, 190 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Persian plum");
	warna.setRgb(112, 28, 28 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Persian red");
	warna.setRgb(204, 51, 51 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Persian rose");
	warna.setRgb(254, 40, 162 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Phlox");
	warna.setRgb(223, 0, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Phthalo blue");
	warna.setRgb(0, 15, 137 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Phthalo green");
	warna.setRgb(18, 53, 36 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Piggy pink");
	warna.setRgb(253, 221, 230 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pine green");
	warna.setRgb(1, 121, 111 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pink");
	warna.setRgb(255, 192, 203 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pink Flamingo");
	warna.setRgb(252, 116, 253 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pink Sherbet");
	warna.setRgb(247, 143, 167 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pink pearl");
	warna.setRgb(231, 172, 207 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pistachio");
	warna.setRgb(147, 197, 114 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Platinum");
	warna.setRgb(229, 228, 226 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Plum");
	warna.setRgb(221, 160, 221 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Portland Orange");
	warna.setRgb(255, 90, 54 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Powder blue");
	warna.setRgb(176, 224, 230 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Princeton orange");
	warna.setRgb(255, 143, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Prussian blue");
	warna.setRgb(0, 49, 83 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Psychedelic purple");
	warna.setRgb(223, 0, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Puce");
	warna.setRgb(204, 136, 153 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Pumpkin");
	warna.setRgb(255, 117, 24 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Purple");
	warna.setRgb(128, 0, 128 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Purple Heart");
	warna.setRgb(105, 53, 156 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Purple Mountain's Majesty");
	warna.setRgb(157, 129, 186 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Purple mountain majesty");
	warna.setRgb(150, 120, 182 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Purple pizzazz");
	warna.setRgb(254, 78, 218 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Purple taupe");
	warna.setRgb(80, 64, 77 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rackley");
	warna.setRgb(93, 138, 168 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Radical Red");
	warna.setRgb(255, 53, 94 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Raspberry");
	warna.setRgb(227, 11, 93 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Raspberry glace");
	warna.setRgb(145, 95, 109 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Raspberry pink");
	warna.setRgb(226, 80, 152 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Raspberry rose");
	warna.setRgb(179, 68, 108 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Raw Sienna");
	warna.setRgb(214, 138, 89 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Razzle dazzle rose");
	warna.setRgb(255, 51, 204 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Razzmatazz");
	warna.setRgb(227, 37, 107 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Red");
	warna.setRgb(255, 0, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Red Orange");
	warna.setRgb(255, 83, 73 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Red brown");
	warna.setRgb(165, 42, 42 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Red violet");
	warna.setRgb(199, 21, 133 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rich black");
	warna.setRgb(0, 64, 64 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rich carmine");
	warna.setRgb(215, 0, 64 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rich electric blue");
	warna.setRgb(8, 146, 208 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rich lilac");
	warna.setRgb(182, 102, 210 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rich maroon");
	warna.setRgb(176, 48, 96 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rifle green");
	warna.setRgb(65, 72, 51 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Robin's Egg Blue");
	warna.setRgb(31, 206, 203 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rose");
	warna.setRgb(255, 0, 127 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rose bonbon");
	warna.setRgb(249, 66, 158 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rose ebony");
	warna.setRgb(103, 72, 70 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rose gold");
	warna.setRgb(183, 110, 121 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rose madder");
	warna.setRgb(227, 38, 54 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rose pink");
	warna.setRgb(255, 102, 204 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rose quartz");
	warna.setRgb(170, 152, 169 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rose taupe");
	warna.setRgb(144, 93, 93 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rose vale");
	warna.setRgb(171, 78, 82 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rosewood");
	warna.setRgb(101, 0, 11 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rosso corsa");
	warna.setRgb(212, 0, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rosy brown");
	warna.setRgb(188, 143, 143 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Royal azure");
	warna.setRgb(0, 56, 168 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Royal blue");
	warna.setRgb(65, 105, 225 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Royal fuchsia");
	warna.setRgb(202, 44, 146 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Royal purple");
	warna.setRgb(120, 81, 169 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ruby");
	warna.setRgb(224, 17, 95 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ruddy");
	warna.setRgb(255, 0, 40 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ruddy brown");
	warna.setRgb(187, 101, 40 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ruddy pink");
	warna.setRgb(225, 142, 150 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rufous");
	warna.setRgb(168, 28, 7 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Russet");
	warna.setRgb(128, 70, 27 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Rust");
	warna.setRgb(183, 65, 14 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sacramento State green");
	warna.setRgb(0, 86, 63 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Saddle brown");
	warna.setRgb(139, 69, 19 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Safety orange");
	warna.setRgb(255, 103, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Saffron");
	warna.setRgb(244, 196, 48 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Saint Patrick Blue");
	warna.setRgb(35, 41, 122 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Salmon");
	warna.setRgb(255, 140, 105 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Salmon pink");
	warna.setRgb(255, 145, 164 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sand");
	warna.setRgb(194, 178, 128 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sand dune");
	warna.setRgb(150, 113, 23 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sandstorm");
	warna.setRgb(236, 213, 64 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sandy brown");
	warna.setRgb(244, 164, 96 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sandy taupe");
	warna.setRgb(150, 113, 23 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sap green");
	warna.setRgb(80, 125, 42 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sapphire");
	warna.setRgb(15, 82, 186 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Satin sheen gold");
	warna.setRgb(203, 161, 53 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Scarlet");
	warna.setRgb(255, 36, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("School bus yellow");
	warna.setRgb(255, 216, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Screamin Green");
	warna.setRgb(118, 255, 122 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sea blue");
	warna.setRgb(0, 105, 148 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sea green");
	warna.setRgb(46, 139, 87 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Seal brown");
	warna.setRgb(50, 20, 20 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Seashell");
	warna.setRgb(255, 245, 238 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Selective yellow");
	warna.setRgb(255, 186, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sepia");
	warna.setRgb(112, 66, 20 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Shadow");
	warna.setRgb(138, 121, 93 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Shamrock");
	warna.setRgb(69, 206, 162 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Shamrock green");
	warna.setRgb(0, 158, 96 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Shocking pink");
	warna.setRgb(252, 15, 192 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sienna");
	warna.setRgb(136, 45, 23 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Silver");
	warna.setRgb(192, 192, 192 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sinopia");
	warna.setRgb(203, 65, 11 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Skobeloff");
	warna.setRgb(0, 116, 116 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sky blue");
	warna.setRgb(135, 206, 235 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sky magenta");
	warna.setRgb(207, 113, 175 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Slate blue");
	warna.setRgb(106, 90, 205 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Slate gray");
	warna.setRgb(112, 128, 144 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Smalt");
	warna.setRgb(0, 51, 153 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Smokey topaz");
	warna.setRgb(147, 61, 65 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Smoky black");
	warna.setRgb(16, 12, 8 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Snow");
	warna.setRgb(255, 250, 250 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Spiro Disco Ball");
	warna.setRgb(15, 192, 252 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Spring bud");
	warna.setRgb(167, 252, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Spring green");
	warna.setRgb(0, 255, 127 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Steel blue");
	warna.setRgb(70, 130, 180 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Stil de grain yellow");
	warna.setRgb(250, 218, 94 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Stizza");
	warna.setRgb(153, 0, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Stormcloud");
	warna.setRgb(0, 128, 128 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Straw");
	warna.setRgb(228, 217, 111 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sunglow");
	warna.setRgb(255, 204, 51 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sunset");
	warna.setRgb(250, 214, 165 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Sunset Orange");
	warna.setRgb(253, 94, 83 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tan");
	warna.setRgb(210, 180, 140 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tangelo");
	warna.setRgb(249, 77, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tangerine");
	warna.setRgb(242, 133, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tangerine yellow");
	warna.setRgb(255, 204, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Taupe");
	warna.setRgb(72, 60, 50 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Taupe gray");
	warna.setRgb(139, 133, 137 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tawny");
	warna.setRgb(205, 87, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tea green");
	warna.setRgb(208, 240, 192 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tea rose");
	warna.setRgb(244, 194, 194 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Teal");
	warna.setRgb(0, 128, 128 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Teal blue");
	warna.setRgb(54, 117, 136 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Teal green");
	warna.setRgb(0, 109, 91 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Terra cotta");
	warna.setRgb(226, 114, 91 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Thistle");
	warna.setRgb(216, 191, 216 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Thulian pink");
	warna.setRgb(222, 111, 161 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tickle Me Pink");
	warna.setRgb(252, 137, 172 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tiffany Blue");
	warna.setRgb(10, 186, 181 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tiger eye");
	warna.setRgb(224, 141, 60 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Timberwolf");
	warna.setRgb(219, 215, 210 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Titanium yellow");
	warna.setRgb(238, 230, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tomato");
	warna.setRgb(255, 99, 71 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Toolbox");
	warna.setRgb(116, 108, 192 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Topaz");
	warna.setRgb(255, 200, 124 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tractor red");
	warna.setRgb(253, 14, 53 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Trolley Grey");
	warna.setRgb(128, 128, 128 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tropical rain forest");
	warna.setRgb(0, 117, 94 );
	this->daftarWarna.push_back(warna);

	warna.setNama("True Blue");
	warna.setRgb(0, 115, 207 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tufts Blue");
	warna.setRgb(65, 125, 193 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tumbleweed");
	warna.setRgb(222, 170, 136 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Turkish rose");
	warna.setRgb(181, 114, 129 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Turquoise");
	warna.setRgb(48, 213, 200 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Turquoise blue");
	warna.setRgb(0, 255, 239 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Turquoise green");
	warna.setRgb(160, 214, 180 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tuscan red");
	warna.setRgb(102, 66, 77 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Twilight lavender");
	warna.setRgb(138, 73, 107 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Tyrian purple");
	warna.setRgb(102, 2, 60 );
	this->daftarWarna.push_back(warna);

	warna.setNama("UA blue");
	warna.setRgb(0, 51, 170 );
	this->daftarWarna.push_back(warna);

	warna.setNama("UA red");
	warna.setRgb(217, 0, 76 );
	this->daftarWarna.push_back(warna);

	warna.setNama("UCLA Blue");
	warna.setRgb(83, 104, 149 );
	this->daftarWarna.push_back(warna);

	warna.setNama("UCLA Gold");
	warna.setRgb(255, 179, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("UFO Green");
	warna.setRgb(60, 208, 112 );
	this->daftarWarna.push_back(warna);

	warna.setNama("UP Forest green");
	warna.setRgb(1, 68, 33 );
	this->daftarWarna.push_back(warna);

	warna.setNama("UP Maroon");
	warna.setRgb(123, 17, 19 );
	this->daftarWarna.push_back(warna);

	warna.setNama("USC Cardinal");
	warna.setRgb(153, 0, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("USC Gold");
	warna.setRgb(255, 204, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ube");
	warna.setRgb(136, 120, 195 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ultra pink");
	warna.setRgb(255, 111, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ultramarine");
	warna.setRgb(18, 10, 143 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Ultramarine blue");
	warna.setRgb(65, 102, 245 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Umber");
	warna.setRgb(99, 81, 71 );
	this->daftarWarna.push_back(warna);

	warna.setNama("United Nations blue");
	warna.setRgb(91, 146, 229 );
	this->daftarWarna.push_back(warna);

	warna.setNama("University of California Gold");
	warna.setRgb(183, 135, 39 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Unmellow Yellow");
	warna.setRgb(255, 255, 102 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Upsdell red");
	warna.setRgb(174, 32, 41 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Urobilin");
	warna.setRgb(225, 173, 33 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Utah Crimson");
	warna.setRgb(211, 0, 63 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Vanilla");
	warna.setRgb(243, 229, 171 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Vegas gold");
	warna.setRgb(197, 179, 88 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Venetian red");
	warna.setRgb(200, 8, 21 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Verdigris");
	warna.setRgb(67, 179, 174 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Vermilion");
	warna.setRgb(227, 66, 52 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Veronica");
	warna.setRgb(160, 32, 240 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Violet");
	warna.setRgb(238, 130, 238 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Violet Blue");
	warna.setRgb(50, 74, 178 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Violet Red");
	warna.setRgb(247, 83, 148 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Viridian");
	warna.setRgb(64, 130, 109 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Vivid auburn");
	warna.setRgb(146, 39, 36 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Vivid burgundy");
	warna.setRgb(159, 29, 53 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Vivid cerise");
	warna.setRgb(218, 29, 129 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Vivid tangerine");
	warna.setRgb(255, 160, 137 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Vivid violet");
	warna.setRgb(159, 0, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Warm black");
	warna.setRgb(0, 66, 66 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Waterspout");
	warna.setRgb(0, 255, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Wenge");
	warna.setRgb(100, 84, 82 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Wheat");
	warna.setRgb(245, 222, 179 );
	this->daftarWarna.push_back(warna);

	warna.setNama("White");
	warna.setRgb(255, 255, 255 );
	this->daftarWarna.push_back(warna);

	warna.setNama("White smoke");
	warna.setRgb(245, 245, 245 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Wild Strawberry");
	warna.setRgb(255, 67, 164 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Wild Watermelon");
	warna.setRgb(252, 108, 133 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Wild blue yonder");
	warna.setRgb(162, 173, 208 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Wine");
	warna.setRgb(114, 47, 55 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Wisteria");
	warna.setRgb(201, 160, 220 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Xanadu");
	warna.setRgb(115, 134, 120 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Yale Blue");
	warna.setRgb(15, 77, 146 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Yellow");
	warna.setRgb(255, 255, 0 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Yellow Orange");
	warna.setRgb(255, 174, 66 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Yellow green");
	warna.setRgb(154, 205, 50 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Zaffre");
	warna.setRgb(0, 20, 168 );
	this->daftarWarna.push_back(warna);

	warna.setNama("Zinnwaldite brown");
	warna.setRgb(44, 22, 8 );
	this->daftarWarna.push_back(warna);

	
}