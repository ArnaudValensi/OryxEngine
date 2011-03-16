#include <boost/test/auto_unit_test.hpp>
#include "OryxColour.h"

using namespace Oryx;

/** Tests the Colour primitve */

//-----------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(ColourConstructor)
{
	Colour c(0.5f,0.6f,0.7f,1.f);
	BOOST_CHECK(c.r == 0.5f);
	BOOST_CHECK(c.g == 0.6f);
	BOOST_CHECK(c.b == 0.7f);
	BOOST_CHECK(c.a == 1.f);
}
//-----------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(Colour8BitConvert)
{
	Colour c(0.5f,0.0f,0.7f,1.f);
	BOOST_CHECK(c.getR8() == static_cast<byte>(floor(0.5f*255.f)));
	BOOST_CHECK(c.getG8() == 0);
	BOOST_CHECK(c.getB8() == static_cast<byte>(floor(0.7f*255.f)));
	BOOST_CHECK(c.getA8() == 255);
}
//-----------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(ColourSaturate)
{
	Colour c(1.5f,-2.f,0.5f,-0.2f);
	Colour c2 = c.saturateCopy();
	
	// make sure c2 is clamped
	BOOST_CHECK(c2.r == 1.f);
	BOOST_CHECK(c2.g == 0.f);
	BOOST_CHECK(c2.b == 0.5f);
	BOOST_CHECK(c2.a == 0.f);
	
	// ensure c is unchanged
	BOOST_CHECK(c.r == 1.5f);
	BOOST_CHECK(c.g == -2.f);
	BOOST_CHECK(c.b == 0.5f);
	BOOST_CHECK(c.a == -0.2f);

	// try plain saturate
	c.saturate();
	BOOST_CHECK(c.r == 1.f);
	BOOST_CHECK(c.g == 0.f);
	BOOST_CHECK(c.b == 0.5f);
	BOOST_CHECK(c.a == 0.f);
}
//-----------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(ColourEquals)
{
	Colour c(0.6f,0.8f,0.3f,0.2f);
	BOOST_CHECK(c == Colour(0.6f,0.8f,0.3f,0.2f));
	BOOST_CHECK(c != Colour(0.7f,0.8f,0.3f,0.2f));
}
//-----------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(ColourAdd)
{
	Colour c(0.1f,0.1f,0.1f,0.1f);
	c += Colour(0.2f,0.3f,0.4f,0.5f);
	BOOST_CHECK(c == Colour(0.1f+0.2f,0.1f+0.3f,0.1f+0.4f,0.1f+0.5f));
}
//-----------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(ColourSubtract)
{
	Colour c(0.9f,0.9f,0.9f,0.8f);
	c -= Colour(0.05f,0.2f,0.2f,0.4f);
	BOOST_CHECK(c == Colour(0.9f-0.05f,0.9f-0.2f,
		0.9f-0.2f,0.8f-0.4f));
}
//-----------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(ColourMultiply)
{
	Colour c(0.1f,0.1f,0.1f,0.1f);
	c *= 5;
	BOOST_CHECK(c == Colour(0.5f,0.5f,0.5f,0.5f));
}
//-----------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(ColourDivide)
{
	Colour c(0.1f,0.1f,0.1f,0.2f);
	c /= 5;
	BOOST_CHECK(c == Colour(0.1f/5.f,0.1f/5.f,0.1f/5.f,0.2f/5.f));
}
//-----------------------------------------------------------------------