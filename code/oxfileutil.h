#ifndef __ox_fileutil_h__
#define __ox_fileutil_h__

// �t�@�C�����[�e�B���e�B

#include <string>

namespace OX {
	class FileUtil {
	public:
		// �t�@�C���p�X����g���q���擾
		//  addDot : �g���q�̑O�ɓ_��t���邩�H
		//  �߂�l : �g���q�i��Fpng��������.png�j�B�g���q�����������ꍇ�͋󕶎�
		static std::string getExtName( const std::string &path, bool addDot = false );

		// �t�@�C���p�X����t�@�C���̃x�[�X�����擾
		//  onlyFileName : �t�@�C���x�[�X���݂̂ɂ���Hfalse�̏ꍇ�̓f�B���N�g�����t�L
		static std::string getBaseName( const std::string &path, bool onlyFileName = true );
	};
}

#endif