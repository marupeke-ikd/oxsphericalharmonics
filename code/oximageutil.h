#ifndef __ox_oximageutil_h__
#define __ox_oximageutil_h__

// イメージユーティリティ
#include <stdint.h>
#include <memory>

namespace OX {
	// イメージブロック
	// RGBAのメモリブロックを保持（内部参照）
	class ImageBlock {
	public:
		struct Body;
		ImageBlock( Body *body = 0 );
		virtual ~ImageBlock();

		// メモリブロックサイズを取得
		uint32_t size() const;

		// メモリブロックを取得
		uint8_t *p() const;

		// イメージがある？
		bool isExist() const;

		// イメージ幅高取得
		uint32_t width() const;
		uint32_t height() const;

		// 1カラーのバイト数を取得
		uint8_t bytePerColor() const;

	protected:
		std::shared_ptr< Body > body_;
	};

	// 出力用ImageBlock
	class ImageBlockCustom : public ImageBlock {
	public:
		ImageBlockCustom( uint32_t w, uint32_t h, uint32_t bytePerColor, uint8_t *data );
		virtual ~ImageBlockCustom();
	};

	// イメージ操作
	class ImageUtil {
	public:
		enum Format {
			BMP,
			PNG,
			JPEG,
			TGA
		};
		// ファイルからImageBlockを作成
		static ImageBlock createImageBlockFromFile( const char* filePath );

		// ImageBlockから画像ファイルに
		//  jpegQuarity : JPEGのクオリティーレベル(0-100)。他の形式では無視。
		static bool createFileFromImageBlock( const ImageBlock &block, const char* filePath, Format format, int jpegQuarity = 100 );
	};
}
#endif