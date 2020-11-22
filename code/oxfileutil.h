#ifndef __ox_fileutil_h__
#define __ox_fileutil_h__

// ファイルユーティリティ

#include <string>

namespace OX {
	class FileUtil {
	public:
		// ファイルパスから拡張子を取得
		//  addDot : 拡張子の前に点を付けるか？
		//  戻り値 : 拡張子（例：pngもしくは.png）。拡張子が無かった場合は空文字
		static std::string getExtName( const std::string &path, bool addDot = false );

		// ファイルパスからファイルのベース名を取得
		//  onlyFileName : ファイルベース名のみにする？falseの場合はディレクトリも付記
		static std::string getBaseName( const std::string &path, bool onlyFileName = true );
	};
}

#endif