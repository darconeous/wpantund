/*
 *
 * Copyright (c) 2016 Nest Labs, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "assert-macros.h"
#include <syslog.h>
#include <errno.h>
#include "SpinelNCPTaskGetNetworkTopology.h"
#include "SpinelNCPInstance.h"
#include "spinel-extra.h"

using namespace nl;
using namespace nl::wpantund;

nl::wpantund::SpinelNCPTaskGetNetworkTopology::SpinelNCPTaskGetNetworkTopology(
	SpinelNCPInstance* instance,
	CallbackWithStatusArg1 cb,
	Type table_type,
	ResultFormat result_format
) : SpinelNCPTask(instance, cb), mType(table_type), mTable(), mResultFormat(result_format)
{
}

int
nl::wpantund::SpinelNCPTaskGetNetworkTopology::parse_child_table(
	const uint8_t *data_in,
	spinel_size_t data_len,
	Table& child_table
) {
	int ret = kWPANTUNDStatus_Ok;

	child_table.clear();

	while (data_len > 0) {
		spinel_ssize_t len = 0;
		const uint8_t *struct_data;
		spinel_size_t struct_len;
		TableEntry child_info;

		len = spinel_datatype_unpack(
			data_in,
			data_len,
			SPINEL_DATATYPE_DATA_WLEN_S,
			&struct_data,
			&struct_len
		);

		require_action(len > 0, bail, ret = kWPANTUNDStatus_Failure);

		ret = parse_child_entry(struct_data, struct_len, child_info);
		require_noerr(ret, bail);

		child_table.push_back(child_info);

		data_in += len;
		data_len -= len;
	}

bail:
	return ret;
}

int
nl::wpantund::SpinelNCPTaskGetNetworkTopology::parse_child_entry(
	const uint8_t *data_in,
	spinel_size_t data_len,
	TableEntry& child_info
) {
	int ret = kWPANTUNDStatus_Ok;
	spinel_ssize_t len = 0;
	const spinel_eui64_t *eui64 = NULL;
	uint8_t mode;

	memset(&child_info, 0, sizeof(child_info));
	child_info.mType = kChildTable;

	len = spinel_datatype_unpack(
		data_in,
		data_len,
		(
			SPINEL_DATATYPE_EUI64_S         // EUI64 Address
			SPINEL_DATATYPE_UINT16_S        // Rloc16
			SPINEL_DATATYPE_UINT32_S        // Timeout
			SPINEL_DATATYPE_UINT32_S        // Age
			SPINEL_DATATYPE_UINT8_S         // Network Data Version
			SPINEL_DATATYPE_UINT8_S         // Link Quality In
			SPINEL_DATATYPE_INT8_S          // Average RSS
			SPINEL_DATATYPE_UINT8_S         // Mode (flags)
			SPINEL_DATATYPE_INT8_S          // Last Rssi
		),
		&eui64,
		&child_info.mRloc16,
		&child_info.mTimeout,
		&child_info.mAge,
		&child_info.mNetworkDataVersion,
		&child_info.mLinkQualityIn,
		&child_info.mAverageRssi,
		&mode,
		&child_info.mLastRssi
	);

	require_action(len > 0, bail, ret = kWPANTUNDStatus_Failure);

	memcpy(child_info.mExtAddress, eui64, sizeof(child_info.mExtAddress));

	child_info.mRxOnWhenIdle = ((mode & kThreadMode_RxOnWhenIdle) != 0);
	child_info.mSecureDataRequest = ((mode & kThreadMode_SecureDataRequest) != 0);
	child_info.mFullFunction = ((mode & kThreadMode_FullFunctionDevice) != 0);
	child_info.mFullNetworkData = ((mode & kThreadMode_FullNetworkData) != 0);

bail:
	return ret;
}

int
nl::wpantund::SpinelNCPTaskGetNetworkTopology::parse_neighbor_table(const uint8_t *data_in, spinel_size_t data_len,
																			 Table& neighbor_table)
{
	int ret = kWPANTUNDStatus_Ok;

	neighbor_table.clear();

	while (data_len > 0)
	{
		spinel_ssize_t len = 0;
		TableEntry neighbor_info;
		const spinel_eui64_t *eui64 = NULL;
		uint8_t mode;
		bool is_child = false;

		memset(&neighbor_info, 0, sizeof(neighbor_info));

		neighbor_info.mType = kNeighborTable;

		len = spinel_datatype_unpack(
			data_in,
			data_len,
			"t("
				SPINEL_DATATYPE_EUI64_S         // EUI64 Address
				SPINEL_DATATYPE_UINT16_S        // Rloc16
				SPINEL_DATATYPE_UINT32_S        // Age
				SPINEL_DATATYPE_UINT8_S         // Link Quality In
				SPINEL_DATATYPE_INT8_S          // Average RSS
				SPINEL_DATATYPE_UINT8_S         // Mode (flags)
				SPINEL_DATATYPE_BOOL_S          // Is Child
				SPINEL_DATATYPE_UINT32_S        // Link Frame Counter
				SPINEL_DATATYPE_UINT32_S        // MLE Frame Counter
				SPINEL_DATATYPE_INT8_S          // Last Rssi
			")",
			&eui64,
			&neighbor_info.mRloc16,
			&neighbor_info.mAge,
			&neighbor_info.mLinkQualityIn,
			&neighbor_info.mAverageRssi,
			&mode,
			&is_child,
			&neighbor_info.mLinkFrameCounter,
			&neighbor_info.mMleFrameCounter,
			&neighbor_info.mLastRssi
		);

		if (len <= 0)
		{
			ret = kWPANTUNDStatus_Failure;
			break;
		}

		memcpy(neighbor_info.mExtAddress, eui64, sizeof(neighbor_info.mExtAddress));

		neighbor_info.mRxOnWhenIdle = ((mode & kThreadMode_RxOnWhenIdle) != 0);
		neighbor_info.mSecureDataRequest = ((mode & kThreadMode_SecureDataRequest) != 0);
		neighbor_info.mFullFunction = ((mode & kThreadMode_FullFunctionDevice) != 0);
		neighbor_info.mFullNetworkData = ((mode & kThreadMode_FullNetworkData) != 0);
		neighbor_info.mIsChild = is_child;

		neighbor_table.push_back(neighbor_info);

		data_in += len;
		data_len -= len;
	}

	return ret;
}

int
nl::wpantund::SpinelNCPTaskGetNetworkTopology::parse_router_table(const uint8_t *data_in, spinel_size_t data_len, Table& router_table)
{
	int ret = kWPANTUNDStatus_Ok;

	router_table.clear();

	while (data_len > 0)
	{
		spinel_ssize_t len = 0;
		const uint8_t *struct_data;
		spinel_size_t struct_len;
		TableEntry router_info;

		len = spinel_datatype_unpack(
			data_in,
			data_len,
			SPINEL_DATATYPE_DATA_WLEN_S,
			&struct_data,
			&struct_len
		);

		require_action(len > 0, bail, ret = kWPANTUNDStatus_Failure);

		ret = parse_router_entry(struct_data, struct_len, router_info);

		require_noerr(ret, bail);

		router_table.push_back(router_info);

		data_in += len;
		data_len -= len;
	}

bail:
	return ret;
}

int
nl::wpantund::SpinelNCPTaskGetNetworkTopology::parse_router_entry(const uint8_t *data_in, spinel_size_t data_len, TableEntry& router_info)
{
	int ret = kWPANTUNDStatus_Ok;
	spinel_ssize_t len = 0;
	const spinel_eui64_t *eui64 = NULL;
	uint8_t age;
	bool link_established = false;

	memset(&router_info, 0, sizeof(router_info));

	router_info.mType = kRouterTable;

	len = spinel_datatype_unpack(
		data_in,
		data_len,
		(
			SPINEL_DATATYPE_EUI64_S         // EUI64 Address
			SPINEL_DATATYPE_UINT16_S        // Rloc16
			SPINEL_DATATYPE_UINT8_S         // Router Id
			SPINEL_DATATYPE_UINT8_S         // Next hop
			SPINEL_DATATYPE_UINT8_S         // Path Cost
			SPINEL_DATATYPE_UINT8_S         // Link Quality In
			SPINEL_DATATYPE_UINT8_S         // Link Quality Out
			SPINEL_DATATYPE_UINT8_S         // Age
			SPINEL_DATATYPE_BOOL_S          // Is Link Established
		),
		&eui64,
		&router_info.mRloc16,
		&router_info.mRouterId,
		&router_info.mNextHop,
		&router_info.mPathCost,
		&router_info.mLinkQualityIn,
		&router_info.mLinkQualityOut,
		&age,
		&link_established
	);

	require_action(len > 0, bail, ret = kWPANTUNDStatus_Failure);

	memcpy(router_info.mExtAddress, eui64, sizeof(router_info.mExtAddress));

	router_info.mAge = age;
	router_info.mLinkEstablished = link_established;

bail:
	return ret;
}

unsigned int
nl::wpantund::SpinelNCPTaskGetNetworkTopology::property_key_for_type(Type type)
{
	unsigned int prop_key = 0;

	switch (type)
	{
	case kChildTable:
		prop_key = SPINEL_PROP_THREAD_CHILD_TABLE;
		break;

	case kNeighborTable:
		prop_key = SPINEL_PROP_THREAD_NEIGHBOR_TABLE;
		break;

	case kRouterTable:
		prop_key = SPINEL_PROP_THREAD_ROUTER_TABLE;
		break;
	}

	return prop_key;
}

int
nl::wpantund::SpinelNCPTaskGetNetworkTopology::vprocess_event(int event, va_list args)
{
	int ret = kWPANTUNDStatus_Failure;
	unsigned int prop_key;
	const uint8_t *data_in;
	spinel_size_t data_len;

	EH_BEGIN();

	if (!mInstance->mEnabled) {
		ret = kWPANTUNDStatus_InvalidWhenDisabled;
		finish(ret);
		EH_EXIT();
	}

	if (mInstance->get_ncp_state() == UPGRADING) {
		ret = kWPANTUNDStatus_InvalidForCurrentState;
		finish(ret);
		EH_EXIT();
	}

	// Wait for a bit to see if the NCP will enter the right state.
	EH_REQUIRE_WITHIN(
		NCP_DEFAULT_COMMAND_RESPONSE_TIMEOUT,
		!ncp_state_is_initializing(mInstance->get_ncp_state()),
		on_error
	);

	// The first event to a task is EVENT_STARTING_TASK. The following
	// line makes sure that we don't start processing this task
	// until it is properly scheduled. All tasks immediately receive
	// the initial `EVENT_STARTING_TASK` event, but further events
	// will only be received by that task once it is that task's turn
	// to execute.
	EH_WAIT_UNTIL(EVENT_STARTING_TASK != event);

	mNextCommand = SpinelPackData(
		SPINEL_FRAME_PACK_CMD_PROP_VALUE_GET,
		property_key_for_type(mType)
	);

	EH_SPAWN(&mSubPT, vprocess_send_command(event, args));

	ret = mNextCommandRet;

	require_noerr(ret, on_error);

	require(EVENT_NCP_PROP_VALUE_IS == event, on_error);

	prop_key = va_arg(args, unsigned int);
	data_in = va_arg(args, const uint8_t*);
	data_len = va_arg_small(args, spinel_size_t);

	require(prop_key == property_key_for_type(mType), on_error);

	if (mType == kChildTable) {
		parse_child_table(data_in, data_len, mTable);
	} else if (mType == kNeighborTable) {
		parse_neighbor_table(data_in, data_len, mTable);
	} else if (mType == kRouterTable) {
		parse_router_table(data_in, data_len, mTable);
	}

	ret = kWPANTUNDStatus_Ok;

	if (mResultFormat == kResultFormat_StringArray)
	{
		std::list<std::string> result;
		Table::iterator it;

		for (it = mTable.begin(); it != mTable.end(); it++)
		{
			result.push_back(it->get_as_string());
		}

		finish(ret, result);
	}
	else if (mResultFormat == kResultFormat_ValueMapArray)
	{
		std::list<ValueMap> result;
		Table::iterator it;

		for (it = mTable.begin(); it != mTable.end(); it++)
		{
			result.push_back(it->get_as_valuemap());
		}

		finish(ret, result);
	}
	else
	{
		finish(ret);
	}

	mTable.clear();

	EH_EXIT();

on_error:

	if (ret == kWPANTUNDStatus_Ok) {
		ret = kWPANTUNDStatus_Failure;
	}

	syslog(LOG_ERR, "Getting child/neighbor table failed: %d", ret);

	finish(ret);

	mTable.clear();

	EH_END();
}

std::string
SpinelNCPTaskGetNetworkTopology::TableEntry::get_as_string(void) const
{
	char c_string[800];

	switch (mType)
	{
	case kChildTable:
		snprintf(c_string, sizeof(c_string),
			"%02X%02X%02X%02X%02X%02X%02X%02X, "
			"RLOC16:%04x, "
			"NetDataVer:%d, "
			"LQIn:%d, "
			"AveRssi:%d, "
			"LastRssi:%d, "
			"Timeout:%u, "
			"Age:%u, "
			"RxOnIdle:%s, "
			"FFD:%s, "
			"SecDataReq:%s, "
			"FullNetData:%s",
			mExtAddress[0], mExtAddress[1], mExtAddress[2], mExtAddress[3],
			mExtAddress[4], mExtAddress[5], mExtAddress[6], mExtAddress[7],
			mRloc16,
			mNetworkDataVersion,
			mLinkQualityIn,
			mAverageRssi,
			mLastRssi,
			mTimeout,
			mAge,
			mRxOnWhenIdle ? "yes" : "no",
			mFullFunction ? "yes" : "no",
			mSecureDataRequest ? "yes" : "no",
			mFullNetworkData ? "yes" : "no"
		);
		break;

	case kNeighborTable:
		snprintf(c_string, sizeof(c_string),
			"%02X%02X%02X%02X%02X%02X%02X%02X, "
			"RLOC16:%04x, "
			"LQIn:%d, "
			"AveRssi:%d, "
			"LastRssi:%d, "
			"Age:%u, "
			"LinkFC:%u, "
			"MleFC:%u, "
			"IsChild:%s, "
			"RxOnIdle:%s, "
			"FFD:%s, "
			"SecDataReq:%s, "
			"FullNetData:%s",
			mExtAddress[0], mExtAddress[1], mExtAddress[2], mExtAddress[3],
			mExtAddress[4], mExtAddress[5], mExtAddress[6], mExtAddress[7],
			mRloc16,
			mLinkQualityIn,
			mAverageRssi,
			mLastRssi,
			mAge,
			mLinkFrameCounter,
			mMleFrameCounter,
			mIsChild ? "yes" : "no",
			mRxOnWhenIdle ? "yes" : "no",
			mFullFunction ? "yes" : "no",
			mSecureDataRequest ? "yes" : "no",
			mFullNetworkData ? "yes" : "no"
		);
		break;

	case kRouterTable:
		snprintf(c_string, sizeof(c_string),
			"%02X%02X%02X%02X%02X%02X%02X%02X, "
			"RLOC16:%04x, "
			"RouterId:%d, "
			"NextHop:%d, "
			"PathCost:%d, "
			"LQIn:%d, "
			"LQOut:%d, "
			"Age:%d, "
			"LinkEst:%s",
			mExtAddress[0], mExtAddress[1], mExtAddress[2], mExtAddress[3],
			mExtAddress[4], mExtAddress[5], mExtAddress[6], mExtAddress[7],
			mRloc16,
			mRouterId,
			mNextHop,
			mPathCost,
			mLinkQualityIn,
			mLinkQualityOut,
			mAge,
			mLinkEstablished ? "yes" : "no"
		);
		break;

	default:
		c_string[0] = 0;
		break;
	}

	return std::string(c_string);
}

ValueMap
SpinelNCPTaskGetNetworkTopology::TableEntry::get_as_valuemap(void) const
{
	ValueMap entryMap;
	uint64_t addr;

	if (mType == kRouterTable) {
		goto bail;
	}

	addr  = (uint64_t) mExtAddress[7];
	addr |= (uint64_t) mExtAddress[6] << 8;
	addr |= (uint64_t) mExtAddress[5] << 16;
	addr |= (uint64_t) mExtAddress[4] << 24;
	addr |= (uint64_t) mExtAddress[3] << 32;
	addr |= (uint64_t) mExtAddress[2] << 40;
	addr |= (uint64_t) mExtAddress[1] << 48;
	addr |= (uint64_t) mExtAddress[0] << 56;

#define SPINEL_TOPO_MAP_INSERT(KEY, VAL) entryMap.insert( std::pair<std::string, boost::any>( KEY, VAL ) )

	SPINEL_TOPO_MAP_INSERT( kWPANTUNDValueMapKey_NetworkTopology_ExtAddress,         addr               );
	SPINEL_TOPO_MAP_INSERT( kWPANTUNDValueMapKey_NetworkTopology_RLOC16,             mRloc16            );
	SPINEL_TOPO_MAP_INSERT( kWPANTUNDValueMapKey_NetworkTopology_LinkQualityIn,      mLinkQualityIn     );
	SPINEL_TOPO_MAP_INSERT( kWPANTUNDValueMapKey_NetworkTopology_AverageRssi,        mAverageRssi       );
	SPINEL_TOPO_MAP_INSERT( kWPANTUNDValueMapKey_NetworkTopology_LastRssi,           mLastRssi          );
	SPINEL_TOPO_MAP_INSERT( kWPANTUNDValueMapKey_NetworkTopology_Age,                mAge               );
	SPINEL_TOPO_MAP_INSERT( kWPANTUNDValueMapKey_NetworkTopology_RxOnWhenIdle,       mRxOnWhenIdle      );
	SPINEL_TOPO_MAP_INSERT( kWPANTUNDValueMapKey_NetworkTopology_FullFunction,       mFullFunction      );
	SPINEL_TOPO_MAP_INSERT( kWPANTUNDValueMapKey_NetworkTopology_SecureDataRequest,  mSecureDataRequest );
	SPINEL_TOPO_MAP_INSERT( kWPANTUNDValueMapKey_NetworkTopology_FullNetworkData,    mFullNetworkData   );

	if (mType == kChildTable) {
	SPINEL_TOPO_MAP_INSERT( kWPANTUNDValueMapKey_NetworkTopology_Timeout,            mTimeout            );
	SPINEL_TOPO_MAP_INSERT( kWPANTUNDValueMapKey_NetworkTopology_NetworkDataVersion, mNetworkDataVersion );
	} else {
	SPINEL_TOPO_MAP_INSERT( kWPANTUNDValueMapKey_NetworkTopology_LinkFrameCounter, mLinkFrameCounter );
	SPINEL_TOPO_MAP_INSERT( kWPANTUNDValueMapKey_NetworkTopology_MleFrameCounter,  mMleFrameCounter  );
	SPINEL_TOPO_MAP_INSERT( kWPANTUNDValueMapKey_NetworkTopology_IsChild,          mIsChild          );
	}

bail:
	return entryMap;
}
