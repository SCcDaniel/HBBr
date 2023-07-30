//---------------------------------------
//ClassName:	HString
//Date:			2020.6.23
//Author:		SCcDaniel
//Explain:		Dream Engine string class.
//---------------------------------------
#pragma once
#include <comutil.h>  
#include <ostream>
#include <math.h>
#include <vector>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#pragma comment(lib, "comsuppw.lib")
class  HString
{
public:
	//�ַ�����ʼ��
	HString()
	{
		this->_str = new char[2];
		this->_str[0] = ' ';
		this->_str[0] = '\0';
		this->length = 1;
	}
	HString(const char* str)
	{
		if (str == nullptr)
			return;
		clear();
		this->length = strlen(str);
		this->_str = new char[this->length + 1];//Ԥ�����һ��'/0'���ַ���λ��
		strcpy_s(this->_str, this->length + 1, str);
	}
	HString(const wchar_t* str)
	{
		if (str == nullptr)
			return;
		clear();
		const char* result = pws2s(str);
		this->length = strlen(result);
		this->_str = new char[this->length + 1];//Ԥ�����һ��'/0'���ַ���λ��
		strcpy_s(this->_str, this->length + 1, result);
	}
	HString(const HString& obj)
	{
		if (obj.Length() > 0)
		{
			clear();
			this->length = obj.Length();
			this->_str = new char[this->length + 1];
			strcpy_s(this->_str, this->length + 1, obj._str);
		}
		else
		{
			clear();
			this->_str = new char[1];
			this->_str[0] = '\0';
			this->length = 0;
		}
	}

	~HString()
	{
		clear();
	}

	void operator=(const HString& obj)
	{
		clear();
		this->length = obj.Length();
		this->_str = new char[this->length + 1];
		strcpy_s(this->_str, this->length + 1, obj._str);
	}

	void operator/(const HString& obj)
	{
		HString newAppend = GetSeparate() + obj;
		this->append(newAppend);
	}

	HString& operator/=(const HString& obj)
	{
		HString newAppend = GetSeparate() + obj;
		this->append(newAppend);
		return *this;
	}

	HString& operator+=(const HString& obj)
	{
		if (obj._str == nullptr || obj._str == NULL)
			return *this;
		char* temp = new char[length + 1];
		strcpy_s(temp, length + 1, _str);//�Ȱ��ַ������Ƶ�temp
		if (_str != NULL)
		{
			delete[] _str;//���
			_str = NULL;
		}
		//�����µĴ�С
		length = length + obj.length;
		_str = new char[length + 1];
		//���ƻ�ȥ
		strcpy_s(_str, strlen(temp) + 1, temp);
		//���Ұ��µ��ַ���������ȥ
		strcat_s(_str, length + 1, obj._str);
		//�ͷ�temp
		delete[] temp;
		temp = NULL;
		length = strlen(_str);
		return *this;
	}

	friend HString operator+(const HString& obj, const HString& obj1)
	{
		if (obj1._str == nullptr || obj1._str == NULL)
			return obj._str;
		size_t len = obj.length + obj1.length;
		char* temp = new char[len + 1];//�½��ַ�����
		strcpy_s(temp, obj.length + 1, obj._str);//���Ƶ�һ��
		strcat_s(temp, len + 1, obj1._str);	//���µ��ַ���������ȥ
		HString result(temp);//��ֵ���µı���
		//�ͷ�temp
		delete[] temp;
		temp = NULL;
		return result;
	}

	//�±�
	char operator[](size_t index)
	{
		return _str[index];
	}

	//����ascii������
	bool operator<(const HString& obj)const
	{
		if (obj._str == nullptr)
		{
			//DE_ASSERT(obj._str == nullptr,"HString Error");
			return false;
		}
		if (obj.length <= 0)
			return false;
		else if (this->length <= 0)
			return true;
		else
		{
			if (strcmp(_str, obj.c_strC()) < 0)
			{
				return true;
			}
		}
		return false;
	}

	bool operator>(const HString& obj)const
	{
		if (obj.length <= 0)
			return false;
		else if (this->length <= 0)
			return true;
		else
		{
			if (strcmp(_str, obj.c_strC()) > 0)
			{
				return true;
			}
		}
		return false;
	}

	bool operator==(const HString& c) const
	{
		return strcmp(this->_str, c._str) == 0;
	}

	bool operator!=(const HString& c) const
	{
		return strcmp(this->_str, c._str) != 0;
	}

	__forceinline HString Left(size_t index)
	{
		if (index >= this->length || index <= 0)
		{
			return *this;
		}
		else
		{
			this->Remove(index, this->length);
		}
		return *this;
	}

	__forceinline HString Right(size_t index)
	{
		if (index >= this->length || index < 0)
		{
			return *this;
		}
		else
		{
			size_t ind = length - index;
			this->Remove(0, index);
		}
		return *this;
	}

	//�����±�ɾ��ĳ���ַ���
	void	Remove(size_t begin, size_t end)
	{
		if (end < begin)
			return;
		else if (end > length)
			end = length;
#if 1
		std::string resultStr = _str;
		resultStr.erase(resultStr.begin() + begin , resultStr.begin() + end);
		this->assign(resultStr.c_str());
#else
		size_t AllLen = length;
		size_t leftLen = begin;//��ȡ֮ǰ�ĳ���
		size_t rightLen = length - (end + 1);//��ȡ֮��ĳ���
		if ((end + 1) - (begin) == length)
		{
			//Ҫɾ�����ַ������͵�ǰ�ַ�����һ�£�ֱ�����
			clear();
			this->_str = new char[1];
			this->_str[0] = '\0';
			this->length = 0;
			return;
		}
		length = leftLen + rightLen;
		char* temp = new char[length + 2];//����0��\0
		size_t index = 0;
		for (size_t i = 0; i < leftLen; i++)
		{
			temp[index] = _str[i];//��ȡ֮ǰ���ַ�
			index += 1;
		}
		for (size_t i = end + 1; i < AllLen; i++)
		{
			temp[index] = _str[i];//��ȡ֮����ַ�
			index += 1;
		}
		temp[length] = '\0';//���һ���ַ������ַ�
		if (_str != NULL)
			delete[] _str;//�ͷ��ַ���
		_str = new char[length + 1];
		strcpy_s(_str, strlen(temp) + 1, temp);
		delete[] temp;
		temp = NULL;
		length = strlen(_str);
#endif
	}

	//����ĳ���ַ���ɾ��
	void Remove(const char* removeStr)
	{
#if 1
		size_t newLen = strlen(removeStr);
		if (newLen <= 0)
			return;
		std::string resultStr = _str;
		std::string::size_type beginPos = resultStr.find(removeStr);
		if(beginPos != std::string::npos)
		{
			resultStr.erase(resultStr.begin() + beginPos , resultStr.begin() + beginPos + newLen);
		}
		this->assign(resultStr.c_str());
#elif 0
		HString result;
		auto splitStr = this->Split(removeStr);
		for (auto s : splitStr)
			result += s;
		this->assign(result);
#else
		size_t len = strlen(removeStr);//��ȡ�ַ����ĳ���
		char* temp = NULL;
		temp = new char[len + 1];//���������������\0�ַ�
		temp[len] = '\0';
		//resetRemove:
		for (size_t i = 0; i < length; i++)
		{
			for (size_t j = 0; j < len; j++)
			{
				temp[j] = _str[i + j];//��ȡ�����ַ�
			}
			if (strcmp(temp, removeStr) == 0)//�ж������ַ����Ƿ�� ������������� ������ͬ����ͬ��ɾ�����ö��ַ�
			{
				size_t oldLen = length;
				length = length - len;
				size_t index = 0;
				char* temp_str = new char[length + 1];
				for (size_t j = 0; j < i; j++)//��ȡ���ַ���
				{
					temp_str[index] = _str[j];
					index++;
				}
				for (size_t j = i + len; j < oldLen; j++)//��ȡ���ַ���
				{
					temp_str[index] = _str[j];
					index++;
				}
				temp_str[length] = '\0';//���һ���ַ������ַ�
				if (_str != NULL)
					delete[] _str;//�ͷ��ַ���
				_str = new char[length + 1];
				strcpy_s(_str, strlen(temp_str) + 1, temp_str);
				_str[length] = '\0';
				delete[] temp_str;
				//goto resetRemove;//��Ϊ�ַ��������˱仯�����¿�ʼѭ��
				if (i <= len)//��Ϊ�ַ��������˱仯����ѭ��ֵi���˵� ��Ӧɾ�����ֵ
					i = 0;
				else
					i -= len;
			}
		}
		delete[] temp;
		temp = NULL;
#endif
	}

	__forceinline void assign(const char* str)
	{
		clear();
		length = strlen(str);
		_str = new char[length + 1];//Ԥ�����һ��'/0'���ַ���λ��
		strcpy_s(_str, length + 1, str);
	}

	__forceinline void assign(HString str)
	{
		clear();
		length = str.length;
		_str = new char[length + 1];//Ԥ�����һ��'/0'���ַ���λ��
		strcpy_s(_str, length + 1, str._str);
	}

	__forceinline size_t	Length()const { return length; }

	__forceinline const char* c_str() { return _str; }

	__forceinline const wchar_t* c_wstr() { return ps2ws(_str); }

	__forceinline const char* c_strC()const { return _str; }

	inline void clear()
	{
		ReleaseCache();
		if (_str != NULL)
		{
			delete[] _str;
			_str = NULL;
		}
		length = 0;
	}

	inline void append(const char* str)
	{
		size_t len = length + strlen(str);
		char* temp = new char[len + 1];
		strcpy_s(temp, length + 1, _str);
		strcat_s(temp, len + 1, str);
		clear();
		_str = temp;
		length = strlen(_str);
	}

	__forceinline void append(const wchar_t* str)
	{
		pws2s(str);
		this->append(m_char);
	}

	inline void append(HString& str)
	{
		size_t len = length + str.length;
		char* temp = new char[len + 1];
		strcpy_s(temp, length + 1, _str);
		strcat_s(temp, len + 1, str._str);
		HString result(temp);
		this->assign(result);
		//�ͷ�temp
		delete[] temp;
		temp = NULL;
	}

	//�ж��ַ��Ƿ���ͬ���ڶ����������� �Ƿ����ִ�С
	inline bool IsSame(HString c, bool strict = true) const
	{
		if (this == nullptr)
			return false;
		if (strict == true)
		{
			if (this->length == c.length)
			{
				if (strcmp(_str, c.c_str()) == 0)
				{
					return true;
				}
				else
					return false;
			}
			else
				return false;
		}
		else
		{
			if (_stricmp(_str, c.c_str()) == 0)
			{
				return true;
			}
		}
		return false;
	}

	inline std::vector<HString> Split(const char* pattern)
	{
		if(strstr(this->_str , pattern) == NULL)
		{
			return{ this->_str };
		}
		size_t patternLeng = strlen(pattern);
		size_t charLen = strlen(this->_str); 
		std::vector<HString> resultVec;
		if (patternLeng > charLen)
			return { this->_str };
		//const char* convert to char*
		char* strc = new char[charLen + 1];
		strcpy_s(strc, charLen + 1, this->_str);
		strc[charLen] = '\0';
		char* temp = NULL;
		char* tmpStr = strtok_s(strc, pattern, &temp);
		while (tmpStr != NULL)
		{
			resultVec.push_back(tmpStr);
			tmpStr = strtok_s(NULL, pattern, &temp);
		}

		delete[] strc;

		return resultVec;
	}

	//��ȡ�ļ���+��׺
	inline HString GetFileName()
	{
		size_t index = 0;
		//�ҵ����һ�� '\'����'/'
		for (size_t i = length - 1; i > 0; i--)
		{
			if (_str[i] == '/' || _str[i] == '\\')
			{
				break;
			}
			index++;
		}
		HString out = *this;
		out.Remove(0, out.length - index);
		return out;
	}

	//��ȡ�ļ���
	HString GetBaseName()
	{
		size_t index = 0;
		HString out = *this;
		bool bFound = false;
		//�ҵ����һ�� '\'����'/'
		for (size_t i = length - 1; i > 0; i--)
		{
			if (_str[i] == '/' || _str[i] == '\\')
			{
				bFound = true;
				break;
			}
			index++;
		}
		if(bFound)
			out.Remove(0, out.length - index);
		//�ҵ����һ�� '.'
		index = 0;
		bFound = false;
		for (size_t i = out.length - 1; i > 0; i--)
		{
			if (out[i] == '.')
			{
				bFound = true;
				break;
			}
			index++;
		}
		if (bFound)
			out.Remove(out.length - index - 1 , out.length);
		return out;
	}

	/* ��ȡ�ļ���׺ */
	inline HString GetSuffix()
	{
#if 0

#else
		size_t index = 0 ;
		bool bFound = false;
		HString out = *this;
		//�ҵ����һ�� '.'
		const auto lenStr = strlen(_str);
		for (size_t i = lenStr - 1; i > 0; i--)
		{
			if (out[i] == '.')
			{
				bFound = true;
				break;
			}
			index++;
		}
		if (bFound)//���ҵ���ɾ���鲻�����ʹ���û�С�
			out.Remove(0, lenStr - index);
		return out;
#endif
	}

	/*  ��ȡ�ļ�·�� */
	inline HString GetFilePath()
	{
		size_t index = 0;
		bool bFound = false;
		auto len = strlen(_str);
		//�ҵ����һ�� '\'����'/'
		for (size_t i = len - 1 ; i > 0; i--)
		{
			if (_str[i] == '/' || _str[i] == '\\')
			{
				bFound = true;
				break;
			}
			index++;
		}
		HString out = *this;
		if (bFound)
			out.Remove(len - index, len);
		return out;
	}

	/*  ·��б�ܸ�ʽ����,Ŀǰwindow��Linux��֧�֡�/��������window�����á�\\���� */
	__forceinline  void  CorrectionPath()
	{
#ifdef WIN32
		this->Replace("/", "\\");
#else
		this->Replace("\\", "/");
#endif
	}

	/* ת���ɴ��ַ�����ʽ��·�� */
	__forceinline void ToPathString()
	{
#ifdef WIN32
		this->Replace("\\", "\\\\");
#else
		//this->Replace("/", "/");
#endif
	}

	__forceinline static HString GetSeparate()
	{
#ifdef WIN32
		return "\\";
#else
		return "/";
#endif
	}

	/* ��ȡexe�ļ�����·�� */
	__forceinline static HString GetExeFullPath()
	{
		wchar_t szPath[1024];
#ifdef _WIN32
		// Windows specific
		GetModuleFileNameW(NULL, szPath, sizeof(szPath));
#else
		// Linux specific
		auto count = readlink("/proc/self/exe", szPath, sizeof(szPath));
		szPath[count] = '\0';
#endif
		HString path = szPath;
		return path;
	}

	/* ��ȡexe�ļ�·�� */
	__forceinline static HString GetExePathWithoutFileName()
	{
		HString path = GetExeFullPath();
		path = path.GetFilePath();
		return path;
	}

	/*
		�滻�ַ���ԭ�����ȷ��룬�ٲ���
	*/
	inline void Replace(const char* whatStr, const char* newStr)
	{
		//std::vector<HString>split = this->Split(whatStr);
		//if (split.size() > 0)
		//{
		//	this->assign(split[0]);
		//	for (size_t i = 1; i < split.size(); i++)
		//	{
		//		this->append(newStr);
		//		this->append(split[i]);
		//	}
		//}
		std::string str = this->_str;
		std::string old_str = whatStr;
		std::string new_str = newStr;
		size_t pos = 0;
		while ((pos = str.find(old_str, pos)) != std::string::npos) {
			str.replace(pos, old_str.length(), new_str);
			pos += new_str.length();
		}
		this->assign(str.c_str());
	}

	/* �ַ������� */
	__forceinline bool Contains(HString whatStr)
	{
		std::string strCache(_str);
		return strCache.find(whatStr.c_str()) != std::string::npos;
	}

	__forceinline char* ToStr()
	{
		return const_cast<char*>(c_str());
	}

	static __forceinline HString	FromFloat(double f, int precise = 6)
	{
		HString out;
		HString format = "%.";
		format += HString::FromInt(precise) + "f";
		char str[128];
		sprintf_s(str, 128, format.c_str(), f);
		out = str;
		return out;
	}

	static __forceinline HString	FromInt(int i)
	{
		HString out;
		char str[128];
		sprintf_s(str, 128, "%d", i);
		out = str;
		return out;
	}

	static __forceinline HString	FromUInt(unsigned int i)
	{
		HString out;
		char str[128];
		sprintf_s(str, 128, "%u", i);
		out = str;
		return out;
	}

	static __forceinline HString	FromSize_t(size_t i)
	{
		HString out;
		char str[256];
		sprintf_s(str, 256, "%zu", i);
		out = str;
		return out;
	}

	static __forceinline HString	FromBool(bool i)
	{
		return i == true ? "true": "false";
	}

	static __forceinline bool ToBool(const char* str)
	{
		return  str[0]== 't'|| str[0] == 'T'? true : false;
	}

	static __forceinline bool ToBool(HString str)
	{
		return  str.IsSame("true",false) ? true : false;
	}

	static __forceinline int ToInt(const char* str)
	{
		return atoi(str);
	}

	static __forceinline int ToInt(HString str)
	{
		return atoi(str.c_str());
	}

	static __forceinline double ToDouble(HString str)
	{
		return atof(str.c_str());
	}

	static __forceinline double ToDouble(const char* str)
	{
		return atof(str);
	}

	static __forceinline long ToLong(HString str)
	{
		return atol(str.c_str());
	}

	static __forceinline long ToLong(const char* str)
	{
		return atol(str);
	}

	static __forceinline long long ToLongLong(const char* str)
	{
		return atoll(str);
	}

	static __forceinline long long ToLongLong(HString str)
	{
		return atoll(str.c_str());
	}

	static char* ws2s(const wchar_t* ws)
	{
		size_t len = WideCharToMultiByte(CP_ACP, 0, ws, (int)wcslen(ws), NULL, 0, NULL, NULL);
		char* out = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, ws, (int)wcslen(ws), out, (int)len, NULL, NULL);
		out[len] = '\0';
		return out;
	}

	static wchar_t* s2ws(const char* s)
	{
		size_t len = MultiByteToWideChar(CP_ACP, 0, s, (int)strlen(s), NULL, 0);
		wchar_t* out = new wchar_t[len + 1];
		MultiByteToWideChar(CP_ACP, 0, s, (int)strlen(s), out, (int)len);
		out[len] = '\0';
		return out;
	}

	//wchar_t �� char�� ת��
	char* pws2s(const wchar_t* ws)
	{
		ReleaseCache();
		size_t len = WideCharToMultiByte(CP_ACP, 0, ws, (int)wcslen(ws), NULL, 0, NULL, NULL);
		m_char = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, ws, (int)wcslen(ws), m_char, (int)len, NULL, NULL);
		m_char[len] = '\0';
		return m_char;
	}
	//char �� wchar_t�� ת��
	wchar_t* ps2ws(const char* s)
	{
		ReleaseCache();
		size_t len = MultiByteToWideChar(CP_ACP, 0, s, (int)strlen(s), NULL, 0);
		m_wchar = new wchar_t[len + 1];
		MultiByteToWideChar(CP_ACP, 0, s, (int)strlen(s), m_wchar, (int)len);
		m_wchar[len] = '\0';
		return m_wchar;
	}

	//std
	std::string ToStdString()const
	{
		return std::string(_str);
	}

private:
	char* _str = NULL;
	size_t length = 0;

	//Cache
	void ReleaseCache()
	{
		if (m_char != NULL)
		{
			delete[] m_char;
			m_char = NULL;
		}
		if (m_wchar != NULL)
		{
			delete[] m_wchar;
			m_wchar = NULL;
		}
	}
	char* m_char = NULL;
	wchar_t* m_wchar = NULL;
};

//hash
namespace std
{
	struct EqualKey
	{
		bool operator() (const HString& s, const HString& q) const noexcept
		{
			return s==q ;
		}
	}; 
	template<>
	struct hash<HString>
	{
		size_t operator() (const HString& s) const noexcept
		{
			return  hash<std::string>()(s.c_strC());
		}
	};
}
