#include "oximageutil.h"
#include "oxfileutil.h"
#include <algorithm>
#include <cctype>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <fstream>

namespace OX {

	struct ImageBlock::Body {
		Body() {}
		virtual ~Body() {}
		uint8_t *block_ = 0;
		uint32_t size_ = 0;
		uint32_t width_ = 0;
		uint32_t height_ = 0;
		uint32_t bytePerColor_ = 0;

	private:
		Body( const Body & ) = delete;
		Body &operator =( const Body & ) = delete;
	};

	ImageBlock::ImageBlock( Body *body ) : body_( body ? body : new Body() ) {}
	ImageBlock::~ImageBlock() {}

	// メモリブロックサイズを取得
	uint32_t ImageBlock::size() const {
		return body_->size_;
	}

	// メモリブロックを取得
	uint8_t *ImageBlock::p() const {
		return body_->block_;
	}

	// イメージがある？
	bool ImageBlock::isExist() const {
		return body_->size_ > 0;
	}

	uint32_t ImageBlock::width() const {
		return body_->width_;
	}

	uint32_t ImageBlock::height() const {
		return body_->height_;
	}

	// 1カラーのバイト数を取得
	uint8_t ImageBlock::bytePerColor() const {
		return body_->bytePerColor_;
	}



	struct ImageBlockCustomBody : public ImageBlock::Body {
		ImageBlockCustomBody() {}
		virtual ~ImageBlockCustomBody() {
			if ( block_ != 0 ) {
				delete[] block_;
			}
		}
	};

	// 出力用ImageBlock
	ImageBlockCustom::ImageBlockCustom( uint32_t w, uint32_t h, uint32_t bytePerColor, uint8_t *data ) : ImageBlock( new ImageBlockCustomBody ) {
		body_->width_ = w;
		body_->height_ = h;
		body_->bytePerColor_ = bytePerColor;
		body_->size_ = w * h * bytePerColor;
		body_->block_ = new uint8_t[ body_->size_ ];
		if ( data )
			memcpy( body_->block_, data, body_->size_ );
	}
	ImageBlockCustom::~ImageBlockCustom() {}
}


namespace {
	class ImageBlockBody : public OX::ImageBlock::Body {
	public:
		ImageBlockBody( unsigned char* data, uint32_t width, uint32_t height, uint32_t bytePerColor ) {
			block_ = data;
			size_ = width * height * bytePerColor;
			width_ = width;
			height_ = height;
			bytePerColor_ = bytePerColor;
		}
		virtual ~ImageBlockBody() {
			if ( block_ != 0 )
				stbi_image_free( block_ );
		}
	};
}

namespace OX {
	// ファイルからRGBAイメージを作成
	ImageBlock ImageUtil::createImageBlockFromFile( const char* filePath ) {
		if ( filePath == 0 )
			return ImageBlock();

		int x, y, n;
		unsigned char* data = stbi_load( filePath, &x, &y, &n, 0 );
		if ( data == 0 )
			return ImageBlock();
		return ImageBlock( new ImageBlockBody( data, x, y, n ) );
	}

	// ImageBlockから画像ファイルに
	bool ImageUtil::createFileFromImageBlock( const ImageBlock &block, const char* filePath, int jpegQuarity ) {
		int res = 0;
		std::string ext = OX::FileUtil::getExtName( filePath );
		if ( ext == "" )
			return false;

		// lowerに
		std::transform( ext.begin(), ext.end(), ext.begin(),
			[]( unsigned char c ) { return std::tolower( c ); } );

		if ( ext == "bmp" ) {
			res = stbi_write_bmp( filePath, block.width(), block.height(), block.bytePerColor(), block.p() );
		} else if ( ext == "png" ) {
			res = stbi_write_png( filePath, block.width(), block.height(), block.bytePerColor(), block.p(), 0 );
		} else if ( ext == "jpg" ) {
			res = stbi_write_jpg( filePath, block.width(), block.height(), block.bytePerColor(), block.p(), jpegQuarity );
		} else if ( ext == "tga" ) {
			res = stbi_write_tga( filePath, block.width(), block.height(), block.bytePerColor(), block.p() );
		}
		return res != 0;
	}
}