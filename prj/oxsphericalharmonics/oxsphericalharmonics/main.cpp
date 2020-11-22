#include <iostream>
#include "cxxopts.hpp"
#include <stdint.h>
#include "oxsphericalharmonics.h"
#include "oximageutil.h"
#include "oxfileutil.h"

int main(int argc, char** argv)
{
	// オプション
	int32_t level = 3;
	std::string fileBaseName("");
	std::string cubeMapFileName("");
	std::string outputParamFileName("");
	bool showProcess = false;
	bool outputAsText = false;
	cxxopts::Options options("oxsphericalharmonics.exe", "OX Spheric Harmonics Parameter Estimation (v1.00)");
	options.add_options()
		("l,level", "SH band level (def=3)", cxxopts::value< int32_t >(level))
		("f,file", "Base file name of src image. ('hoge.bmp' -> hoge_l.bmp, hoge_r.bmp and so on.", cxxopts::value< std::string >(fileBaseName))
		("o,output", "Output file name of estimated parameter (hoge.dat)", cxxopts::value< std::string >( outputParamFileName ) )
		("t,text", "Output estimated parameter as text (option)", cxxopts::value< bool >( outputAsText ) )
		("c,cubemap", "Output file name of test cube map (option) ('cubemap.bmp')", cxxopts::value< std::string >( cubeMapFileName ) )
		("p,proc", "Show estimate process (option, def=false)", cxxopts::value< bool >( showProcess ) )
		("h,help", "Print help")
		;
	auto res = options.parse(argc, argv);

	if (res.count("help")) {
		std::cout << options.help({}) << std::endl;
		return 0;
	}

	// levelは0以上10以下
	level = (level < 0 ? 3 : level);
	if (level > 10) {
		std::cout << "Maximum SH band level is 10." << std::endl;
		return -1;
	}

	// 入力ベースファイル名が無い場合はエラー
	if (!res.count("f")) {
		std::cout << "need src file base name. (-f)" << std::endl;
		return -1;
	}

	// 出力ファイル名が無い場合はエラー
	if (!res.count("o")) {
		std::cout << "need output file name. (-o)" << std::endl;
		return -1;
	}

	using namespace OX::SphericalHarmonics;

	// 入力ファイル名を作成しファイルをチェック
	std::vector< std::string > fileNames;
	const char* suffix[] = {
		"_px", "_nx", "_py", "_ny", "_pz", "_nz"
	};
	std::string ext = OX::FileUtil::getExtName( fileBaseName, true );
	fileBaseName = OX::FileUtil::getBaseName( fileBaseName, false );
	for ( int32_t i = 0; i < 6; ++i ) {
		fileNames.push_back( fileBaseName + suffix[ i ] + ext );
	}

	// 指定キューブマップファイルを取り込み
	CubeDataFromImage cubeData;
	Error err = cubeData.initialize( fileNames );
	if (err.error_ == true) {
		// 読み込みエラー
		std::cout << "failed to create cube data object.\n" << err.reason_ << std::endl;
		return -1;
	}

	// パラメータ推定
	printf( "Estimate SH parameters from %s.\n", fileBaseName.c_str() );
	printf( " level=%u, output as %s\n", level, outputAsText ? "text" : "binary" );
	CubeEstimater cubeEst( level );
	Result shRes;
	err = cubeEst.estimate( &cubeData, shRes, [ showProcess ]( uint64_t count, uint64_t procCount ) {
		if ( showProcess && count % ( procCount / 40 ) == 0 ) {
			printf( "Param  %llu / %llu\n", count, procCount );
		}
	} );
	if ( err.error_ ) {
		std::cout << "estimate error: " << err.reason_ << std::endl;
		return -1;
	}

	// パラメータ出力
	printf( "Output parameters.\n" );
	std::shared_ptr< OutputResult > output( outputAsText ? new OutputResultText : new OutputResult );
	output->output( shRes, outputParamFileName.c_str() );

	// テストキューブマップ出力
	if ( cubeMapFileName != "" ) {
		printf( "Output cubemap.\n" );
		auto imageBlocks = createCubeMapFromParameters( shRes, 128, CubeMapType::Horizontal_Cross, [ showProcess ]( uint64_t count, uint64_t procCount ) {
			if ( showProcess && count % ( procCount / 40 ) == 0 ) {
				printf( "CubeMap  %llu / %llu\n", count, procCount );
			}
		} );
		OX::ImageUtil::createFileFromImageBlock( imageBlocks[ 0 ], cubeMapFileName.c_str(), OX::ImageUtil::BMP );
	}

	return 0;
}
