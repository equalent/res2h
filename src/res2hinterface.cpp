#include "res2hinterface.hpp"

#include <fstream>


std::map<std::string, Res2h::ResourceEntry> Res2h::resourceMap;


bool Res2h::loadArchive(const std::string & archivePath)
{
    //check if there are entries for this archive already in the map and delete them
    std::map<std::string, ResourceEntry>::iterator rmIt = resourceMap.begin();
    while (rmIt != resourceMap.end()) {
        if (rmIt->second.archivePath == archivePath) {
            //'tis from this archive. erase.
            rmIt = resourceMap.erase(rmIt);
        }
        else {
            ++rmIt;
        }
    }
    //open archive
    std::ifstream inStream;
    inStream.open(archivePath, std::ifstream::in | std::ifstream::binary);
    if (inStream.is_open() && inStream.good()) {
        //worked. read magic bytes
        unsigned char magicBytes[8] = {0};
        inStream.read(reinterpret_cast<char *>(&magicBytes), sizeof(magicBytes));
        std::string magicString(reinterpret_cast<char *>(&magicBytes), sizeof(magicBytes));
        if (magicString == "res2hbin") {
            //magic bytes ok. control checksum. close first for calling checksum function
            inStream.close();
            uint32_t calculatedChecksum = calculateAdler32(archivePath);
            //re-open again
            inStream.open(archivePath, std::ifstream::in | std::ifstream::binary);
            //read checksum from end of file
            inStream.seekg(sizeof(uint32_t), std::ios::end);
            uint32_t fileChecksum = 0;
            inStream.read(reinterpret_cast<char *>(&fileChecksum), sizeof(uint32_t));
            if (fileChecksum == calculatedChecksum) {
                //nice. checksum is ok. rewind and skip behind magic bytes
                inStream.seekg(sizeof(magicBytes), std::ios::beg);
                //check file version
                uint32_t fileVersion = 0;
                inStream.read(reinterpret_cast<char *>(&fileVersion), sizeof(uint32_t));
                if (fileVersion <= RES2H_ARCHIVE_VERSION) {
                    //file version ok. skip currently unused flags
                    inStream.seekg(sizeof(uint32_t), std::ios::cur);
                    //read number of directory entries
                    uint32_t nrOfDirectoryEntries = 0;
                    inStream.read(reinterpret_cast<char *>(&fileVersion), sizeof(uint32_t));
                    if (nrOfDirectoryEntries > 0) {
                        for (uint32_t i = 0; i < nrOfDirectoryEntries; ++i) {
                            //read directory to map
                            ResourceEntry temp;
                            //read size of name
                            uint32_t sizeOfName = 0;
                            inStream.read(reinterpret_cast<char *>(&sizeOfName), sizeof(uint32_t));
                            //read name itself
                            temp.fileName.resize(sizeOfName);
                            inStream.read(reinterpret_cast<char *>(&temp.fileName[0]), sizeOfName - 1);
                            //clear data pointer
                            temp.data = nullptr;
                            //skip currently unused flags
                            inStream.seekg(sizeof(uint32_t), std::ios::cur);
                            //read size of data
                            inStream.read(reinterpret_cast<char *>(&temp.size), sizeof(uint32_t));
                            //read offset to start of data
                            inStream.read(reinterpret_cast<char *>(&temp.dataOffset), sizeof(uint32_t));
                            //read data checksum
                            inStream.read(reinterpret_cast<char *>(&temp.checksum), sizeof(uint32_t));
                            //add archive path
                            temp.archivePath = archivePath;
                            //add to map
                            resourceMap[temp.fileName] = temp;
                        }
                        //close file
                        inStream.close();
                        return true;
                    }
                    else {
                        throw Res2hException(std::string("loadArchive() - Archive \"") + archivePath + "\" contains no files!");
                    }
                }
                else {
                    throw Res2hException(std::string("loadArchive() - Archive \"") + archivePath + "\" has a newer file version and can not be read!");
                }
            }
            else {
                throw Res2hException(std::string("loadArchive() - Archive \"") + archivePath + "\" has a bad checksum!");
            }
        }
        else {
            throw Res2hException(std::string("loadArchive() - File \"") + archivePath + "\" doesn't seem to be a res2h archive!");
        }
    }
    else {
        throw Res2hException(std::string("loadArchive() - Failed to open archive \"") + archivePath + "\".");
    }
	return false;
}

Res2h::ResourceEntry Res2h::loadFile(const std::string & filePath, bool keepInCache)
{
	ResourceEntry temp;
	return temp;
}

void Res2h::releaseCache()
{
    //erase all allocated data in resource map
    std::map<std::string, ResourceEntry>::iterator rmIt = resourceMap.begin();
    while (rmIt != resourceMap.end()) {
        if (rmIt->second.data) {
            //shared_ptr is valid, so there is allocated data. release our reference to it.
            rmIt->second.data.reset();
        }
        ++rmIt;
    }
}

uint32_t Res2h::calculateAdler32(const std::string & filePath, uint32_t adler)
{
	//open file
	std::ifstream inStream;
	inStream.open(filePath, std::ifstream::in | std::ifstream::binary);
	if (inStream.is_open() && inStream.good()) {
		//build checksum
		uint32_t s1 = adler & 0xffff;
		uint32_t s2 = (adler >> 16) & 0xffff;
		//loop until EOF
		while (!inStream.eof() && inStream.good()) {
			char buffer[1024];
			std::streamsize readSize = sizeof(buffer);
			try {
				//try reading data from input file
				inStream.read(reinterpret_cast<char *>(&buffer), sizeof(buffer));
			}
			catch (std::ios_base::failure) {
				//reading didn't work properly. store how many bytes were actually read
				readSize = inStream.gcount();
			}
			//calculate checksum for buffer
			for (std::streamsize n = 0; n < readSize; n++) {
				s1 = (s1 + buffer[n]) % 65521;
				s2 = (s2 + s1) % 65521;
			}
		}
		//close file
		inStream.close();
		//build final checksum
		return (s2 << 16) + s1;
	}
	else {
		throw Res2hException(std::string("calculateAdler32() - Failed to open file \"") + filePath + "\".");
	}
	return adler;
}

//-----------------------------------------------------------------------------

Res2hException::Res2hException(const char * errorString) throw()
	: exception(), error(errorString)
{
}

Res2hException::Res2hException(const std::string & errorString) throw()
	: exception(), error(errorString)
{
}

const char * Res2hException::what() const throw()
{
	return error.c_str();
}

const std::string & Res2hException::whatString() const throw()
{
	return error;
}