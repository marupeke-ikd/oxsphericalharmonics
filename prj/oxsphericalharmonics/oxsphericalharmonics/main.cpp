#include <iostream>
#include "cxxopts.hpp"
#include <stdint.h>
#include "oxsphericalharmonics.h"
#include "oximageutil.h"

int main( int argc, char** argv )
{
	// オプション
	int32_t level = 3;
	std::string fileBaseName( "" );
	std::string ext( "" );
	cxxopts::Options options( "oxsphericalharmonics.exe", "OX Spheric Harmonics Parameter Estimation (v1.00)" );
	options.add_options()
		( "l,level", "SH band level (def=3)", cxxopts::value< int32_t >( level ) )
		( "f,file", "Base file name of src image. ('hoge.bmp' -> hoge_l.bmp, hoge_r.bmp and so on.", cxxopts::value< std::string >( fileBaseName ) )
		( "o,output", "Output file name (hoge.dat)" )
		( "h,help", "Print help" )
		;
	auto res = options.parse( argc, argv );

	if ( res.count( "help" ) ) {
		std::cout << options.help( {} ) << std::endl;
		return 0;
	}

	// levelは1以上10以下
	level = ( level <= 0 ? 3 : level );
	level = ( level > 10 ? 10 : level );

	// 入力ベースファイル名が無い場合はエラーに
	if ( !res.count( "f" ) ) {
		std::cout << "need src file base name. (-f)" << std::endl;
		return -1;
	}

	// 出力ファイル名が無い場合はエラーに
	if ( !res.count( "o" ) ) {
		std::cout << "need output file name. (-o)" << std::endl;
		return -1;
	}

	using namespace OX::SphericalHarmonics;

	std::vector< std::string > fileNames;
	
	fileNames.push_back( "sample04.bmp" );
	fileNames.push_back( "sample04.bmp" );
	fileNames.push_back( "sample04.bmp" );
	fileNames.push_back( "sample04.bmp" );
	fileNames.push_back( "sample04.bmp" );
	fileNames.push_back( "sample04.bmp" );
	/*	fileNames.push_back( "px_128.png" );
	fileNames.push_back( "nx_128.png" );
	fileNames.push_back( "py_128.png" );
	fileNames.push_back( "ny_128.png" );
	fileNames.push_back( "pz_128.png" );
	fileNames.push_back( "nz_128.png" );
*/	CubeDataFromImage cubeData;
	Error err = cubeData.initialize( fileNames );

	printf( "Estimate SH parameters.\n" );
	CubeEstimater cubeEst( 15 );
	Result shRes;
	err = cubeEst.estimate( &cubeData, shRes, []( uint64_t count, uint64_t procCount ) {
		if ( count % ( procCount / 40 ) == 0 ) {
			printf( "Param  %llu / %llu\n", count, procCount );
		}
	} );
	if ( err.error_ ) {
		std::cout << "estimate error: " << err.reason_ << std::endl;
		return -1;
	}

	printf( "Output cubemap.\n" );
	auto imageBlocks = createCubeMapFromParameters( shRes, 128, CubeMapType::Horizontal_Cross, []( uint64_t count, uint64_t procCount ) {
		if ( count % ( procCount / 40 ) == 0 ) {
			printf( "CubeMap  %llu / %llu\n", count, procCount );
		}
	} );

	OX::ImageUtil::createFileFromImageBlock( imageBlocks[ 0 ], "cubemap.bmp", OX::ImageUtil::BMP );
/*
	OX::ImageUtil::createFileFromImageBlock( imageBlocks[ 0 ], "hoge_r.bmp", OX::ImageUtil::BMP );
	OX::ImageUtil::createFileFromImageBlock( imageBlocks[ 1 ], "hoge_l.bmp", OX::ImageUtil::BMP );
	OX::ImageUtil::createFileFromImageBlock( imageBlocks[ 2 ], "hoge_u.bmp", OX::ImageUtil::BMP );
	OX::ImageUtil::createFileFromImageBlock( imageBlocks[ 3 ], "hoge_d.bmp", OX::ImageUtil::BMP );
	OX::ImageUtil::createFileFromImageBlock( imageBlocks[ 4 ], "hoge_f.bmp", OX::ImageUtil::BMP );
	OX::ImageUtil::createFileFromImageBlock( imageBlocks[ 5 ], "hoge_b.bmp", OX::ImageUtil::BMP );
*/
	return 0;
}
