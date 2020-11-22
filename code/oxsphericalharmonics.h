#ifndef __ox_oxsphericalharmonics_h__
#define __ox_oxsphericalharmonics_h__

// 球面調和関数

#include <string>
#include <vector>
#include <functional>
#include "oximageutil.h"

namespace OX {
	namespace SphericalHarmonics {

		// RGBA
		struct RGBA {
			uint8_t r_ = 0;
			uint8_t g_ = 0;
			uint8_t b_ = 0;
			uint8_t a_ = 0;
			RGBA() {}
			RGBA( uint8_t r, uint8_t g, uint8_t b, uint8_t a ) : r_( r ), g_( g ), b_( b ), a_( a ) {}
			double dr() const { return r_ / 255.0; };
			double dg() const { return g_ / 255.0; };
			double db() const { return b_ / 255.0; };
			double da() const { return a_ / 255.0; };
		};

		// 球形データ
		class SphereData {
		public:
			SphereData() {}
			virtual ~SphereData() {}

			// 指定の角度に対する値を取得
			//  th  : 緯度角θ(0～π)
			//  phi : 経度角φ(0～2π)
			virtual double getValue( double th, double phi ) = 0;
		};

		// キューブマップデータ
		class CubeData {
		public:
			enum Face {
				PX,	// X+
				NX,	// X-
				PY,	// Y+
				NY,	// Y-
				PZ,	// Z+
				NZ,	// Z-
				Face_Num
			};

			// 指定のUV位置に対するXYZ座標を取得
			static void getXYZ( Face face, int32_t w, int32_t tu, int32_t tv, double &x, double &y, double &z );

			// 指定のUV位置に対する極座標を取得
			// 戻り値 : 指定UVまでの距離
			static double getPolar( Face face, int32_t w, int32_t tu, int32_t tv, double &th, double &phi );

			CubeData() {}
			virtual ~CubeData() {}

			// マップのテクセルサイズを取得
			virtual uint32_t getTexelSize() const = 0;

			// 指定のUV位置に対する値を取得
			virtual RGBA getValue( Face face, int32_t u, int32_t v ) const = 0;

			// 指定のUV位置に対するXYZ座標を取得
			void getXYZ( Face face, int32_t tu, int32_t tv, double &x, double &y, double &z ) const;

			// 指定のUV位置に対する極座標を取得
			// 戻り値 : 指定UVまでの距離
			double getPolar( Face face, int32_t u, int32_t v, double &th, double &phi ) const;
		};

		// パラメータ
		class Parameter {
		public:
			bool operator ==( const Parameter &r ) const {
				return ( l_ == r.l_ && m_ == r.m_ );
			}
			bool operator <( const Parameter &r ) const {
				if ( l_ < r.l_ ) return true;
				else if ( l_ > r.l_ ) return false;
				return m_ < r.m_;
			}

		public:
			Parameter() {}
			Parameter( uint32_t l, int32_t m, double value ) : l_( l ), m_( m ), value_( value ) {}

			// インデックスをパラメータに変更
			static void toLM( uint32_t idx, uint32_t &l, int32_t &m );

			// パラメータをインデックスに変更
			static uint32_t toIdx( uint32_t l, int32_t m );

			// level取得
			uint32_t l() const;

			// m取得
			int32_t m() const;

			// 値取得
			double value() const;

			// 有効なパラメータ？
			bool isValid() const;

		private:
			const static uint32_t invalidParam_g = 0xffffffff;
			uint32_t l_ = invalidParam_g;	// order
			int32_t m_ = 0;		// index in order( -l <= m <= l )
			double value_ = 0.0;	// 推定値
		};

		// 推定状態
		enum ResultState {
			RS_OK,				// 推定成功
			RS_NO_ESTIMATE,		// 推定前
			RS_INVALID_DATA,	// 入力データが不正
			RS_INVALID_PARAM,	// 入力パラメータが不正
		};

		// カラー
		enum ColorType {
			ColorType_R,
			ColorType_G,
			ColorType_B,
			ColorType_A
		};

		// 推定結果
		class Result {
		public:
			Result() {}
			Result( uint32_t maxLevel, const std::vector< std::vector< Parameter > > &params ) : maxLevel_( maxLevel ), paramsVec_( params ) {}
			~Result() {}

			// 推定状態を取得
			ResultState getState() const;

			// 推定時の最大Levelを取得
			uint32_t getMaxLevel() const;

			// パラメータリスト取得
			const std::vector< Parameter > &getParamList( ColorType ctype ) const;

			// 指定インデックスのパラメータを取得
			//  戻り値 : 無効なパラメータが指定されていた場合はParameter::isValidがfalse
			Parameter getParam( ColorType ctype, uint32_t l, int32_t m ) const;

		private:
			uint32_t maxLevel_ = 0;
			std::vector< std::vector< Parameter > > paramsVec_;	// 推定パラメータ（色別）
			ResultState state_ = ResultState::RS_NO_ESTIMATE;	// 推定状態
		};

		// エラー
		struct Error {
			bool error_ = false;	// trueでエラーあり
			std::string reason_;
			Error() {}
			Error( const std::string &reason ) : error_( true ), reason_( reason ) {}
		};

		// 推定ベース
		class Estimater {
		public:
			Estimater( uint32_t maxLevel ) : maxLevel_( maxLevel ) {}
			virtual ~Estimater() {}

			// 推定時のband order levelの最大値を取得
			uint32_t getMaxLevel() const;

		protected:
			uint32_t maxLevel_;	// 球面調和関数のband orderの最大値
		};

		// 球状データからのパラメータ推定
		class SphereEstimater : public Estimater {
		public:
			using Estimater::Estimater;
			virtual ~SphereEstimater() {}

			// 推定
			//  戻り値 : エラーが発生した場合は有効文字列が返る
			Error estimate( const SphereData *sphere );
		};

		// キューブデータからのパラメータ推定
		class CubeEstimater : public Estimater {
		public:
			using Estimater::Estimater;
			virtual ~CubeEstimater() {}

			// 推定
			Error estimate( const CubeData *cube, Result &res, const std::function< void( uint64_t count, uint64_t procCount ) > &proc );
		};

		// CubeMapイメージからCubeData
		class CubeDataFromImage : public CubeData {
		public:
			using CubeData::CubeData;
			virtual ~CubeDataFromImage() {}

			// 初期化
			//  fileNames : 6面のファイル名(右、左、前、後、上、下の順)
			Error initialize( const std::vector< std::string > &fileNames );

			// 指定のUV位置に対する値を取得
			virtual RGBA getValue( Face face, int32_t u, int32_t v ) const;

			// マップのテクセルサイズを取得
			virtual uint32_t getTexelSize() const override;

		private:
			ImageBlock images_[ 6 ];
		};

		// パラメータ出力
		class OutputResult {
		public:
			OutputResult();
			virtual ~OutputResult();
			virtual Error output( const Result &result, const char* filePath );

		private:
			struct Header {
				uint32_t hederSize_ = 0;		// ヘッダーサイズ
				uint32_t maxOrderLevel_ = 0;
				uint32_t componentListNum_ = 0;	// 各色のリスト数
				uint32_t containAlpha_ = 0;		// αがある場合は1
				uint32_t reserved_ = 0;			// 予約領域
			};
		};

		class OutputResultText : public OutputResult {
		public:
			OutputResultText();
			virtual ~OutputResultText();
			virtual Error output( const Result& result, const char* filePath );
		};

		// 球面調和関数群を作成
		std::vector< std::function< double( double th, double phi )> > createSphericalHarmonicsFuncs( uint32_t level );

		// 推定パラメータからキューブマップ作成
		enum CubeMapType {
			Horizontal_Cross,	// 横クロス
			Vertical_Cross,		// 縦クロス
			Separable,			// 6面分割
		};
		std::vector< ImageBlock > createCubeMapFromParameters( const Result &res, uint32_t width, CubeMapType mapType, const std::function< void( uint64_t count, uint64_t procCount ) > &proc );
	}
}

#endif
