#include "oxfileutil.h"


namespace OX {
	// �t�@�C���p�X����g���q���擾
	//  addDot : �g���q�̑O�ɓ_��t���邩�H
	//  �߂�l : �g���q�i��Fpng��������.png�j�B�g���q�����������ꍇ�͋󕶎�
	std::string FileUtil::getExtName( const std::string& path, bool addDot ) {
		if ( path == "" )
			return std::string();
		size_t extPos = path.find_last_of( "." );
		if ( extPos == std::string::npos ) {
			return std::string();
		}
		return path.substr( extPos + ( addDot ? 0 : 1 ) );
	}

	// �t�@�C���p�X����t�@�C���̃x�[�X�����擾
	std::string FileUtil::getBaseName( const std::string& path, bool onlyFileName ) {
		if ( path == "" )
			return std::string();
		size_t extPos = path.find_last_of( "." );
		size_t dirPos = 0;
		size_t count = extPos;
		if ( onlyFileName == true ) {
			size_t backslashPos = path.find_last_of( "\\" );
			size_t slashPos = path.find_last_of( "/" );
			if ( backslashPos != std::string::npos && slashPos != std::string::npos )
				dirPos = ( backslashPos > slashPos ? backslashPos : slashPos );
			else
				dirPos = ( backslashPos == std::string::npos ? slashPos : backslashPos );
			if ( dirPos == std::string::npos )
				dirPos = 0;
			else {
				dirPos += 1;
				count = ( extPos != std::string::npos ? extPos - dirPos : extPos );
			}
		}
		return path.substr( dirPos, count );
	}
}