#ifndef __ox_oximageutil_h__
#define __ox_oximageutil_h__

// �C���[�W���[�e�B���e�B
#include <stdint.h>
#include <memory>

namespace OX {
	// �C���[�W�u���b�N
	// RGBA�̃������u���b�N��ێ��i�����Q�Ɓj
	class ImageBlock {
	public:
		struct Body;
		ImageBlock( Body *body = 0 );
		virtual ~ImageBlock();

		// �������u���b�N�T�C�Y���擾
		uint32_t size() const;

		// �������u���b�N���擾
		uint8_t *p() const;

		// �C���[�W������H
		bool isExist() const;

		// �C���[�W�����擾
		uint32_t width() const;
		uint32_t height() const;

		// 1�J���[�̃o�C�g�����擾
		uint8_t bytePerColor() const;

	protected:
		std::shared_ptr< Body > body_;
	};

	// �o�͗pImageBlock
	class ImageBlockCustom : public ImageBlock {
	public:
		ImageBlockCustom( uint32_t w, uint32_t h, uint32_t bytePerColor, uint8_t *data );
		virtual ~ImageBlockCustom();
	};

	// �C���[�W����
	class ImageUtil {
	public:
		enum Format {
			BMP,
			PNG,
			JPEG,
			TGA
		};
		// �t�@�C������ImageBlock���쐬
		static ImageBlock createImageBlockFromFile( const char* filePath );

		// ImageBlock����摜�t�@�C����
		//  jpegQuarity : JPEG�̃N�I���e�B�[���x��(0-100)�B���̌`���ł͖����B
		static bool createFileFromImageBlock( const ImageBlock &block, const char* filePath, Format format, int jpegQuarity = 100 );
	};
}
#endif