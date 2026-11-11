#pragma once

#include "network/Types.hpp"
#include <list>
#include <vector>
#include <string>

namespace JE
{
	namespace Resources
	{
		struct ResourceData;
		struct ResourceCacheNode;

		struct ResourceDescriptor
		{
			Net::UInt32 flags;
			Net::UInt32 ordinal;
			Net::UInt32 type;
			std::string name;
			std::string path;
			// Actual cache-list node token at+44; default construction leaves it
			// unwritten. A supplied list iterator would incorrectly initializeNULL.
			ResourceCacheNode* cached;
		};

		class ResourceIndex
		{
		public:
			ResourceIndex(); //50C980; zero bucket count after empty vector construction.
			void Initialize(Net::UInt32 buckets); //50C9E0; resize before publishing count.
			ResourceDescriptor* Find(const char* szName, Net::UInt32 nType); // 0x50BB00
			void Remove(const char* szName, Net::UInt32 nType); // 0x50BDD0; EAX unused.
			void Upsert(const char* szName, Net::UInt32 nType, Net::UInt32 nFlags,
				const char* szPath, Net::UInt32 nOrdinal); // 0x50BF30

		private:
			typedef std::list<ResourceDescriptor> Bucket;
			std::vector<Bucket> buckets;
			Net::UInt32 bucketCount; //+10, zero at construction; Initialize publishes after resize.
			Net::UInt32 Hash(const char* name, Net::UInt32 type) const; //50BA50; ownership unresolved.
		};

		namespace ResourceIndexServices
		{
			// Actual string assignment; exposed only to compare the caller's update order.
			void AssignPath(std::string& destination, const char* szPath);
		}
	}
}
