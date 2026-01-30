#include "MedConverter.hh"

// Function to process header words and then the data
void MiniballMedConverter::ProcessEvent( unsigned long nblock ){

	// Get number of subevents in the data and loop over them
	ndata = ev->GetNumberOfSubEvents();
	for( unsigned int i = 0; i < ndata; i++ ){

		// Pointer to this subevent
		mbs_sevt = ev->GetSubEvent(i);
		if( !mbs_sevt ) continue;

		//std::cout << "Event ID: " << std::dec << ev->GetEventID();
		//std::cout << ", Sub-event: " << mbs_sevt->GetSubEventID();
		//std::cout << std::endl << "Sub-event type: 0x" << std::hex;
		//std::cout << mbs_sevt->GetSubEventType() << ", ";
		//std::cout << mbs_sevt->GetSubEventDescription();
		//std::cout << std::dec << std::endl;


		// Normal DGF data
		if( set->CheckVmeModuleIsDgf( mbs_sevt->GetModuleID() ) )
			ProcessDgfData();

		// DGF Scalers
		else if( set->CheckVmeModuleIsDgfScaler( mbs_sevt->GetModuleID() ) )
			ProcessDgfScaler();

		// General ADCs
		else if( set->CheckVmeModuleIsAdc( mbs_sevt->GetModuleID() ) ) {

			// Mesytec ADCs
			if( mbs_sevt->GetSubEventType() == MBS_STYPE_VME_MADC_1 ||
			   mbs_sevt->GetSubEventType() == MBS_STYPE_VME_MADC_2 ||
			   mbs_sevt->GetSubEventType() == MBS_STYPE_VME_MADC_3 )
				ProcessMesytecAdcData();

			// Assume it is otherwise a CAEN ADC
			else ProcessCaenAdcData();

		}

		// Pattern unit
		else if( set->CheckVmeModuleIsPattern( mbs_sevt->GetModuleID() ) )
			ProcessPatternUnitData();

		// Scaler data
		else if( set->CheckVmeModuleIsScaler( mbs_sevt->GetModuleID() ) )
			ProcessScalerData();

		// Timestamp data from MBS
		else if( mbs_sevt->GetSubEventType() == MBS_STYPE_TIME_STAMP )
			continue;

		// Otherwise print an error for now (expand more types later)
		else {

			std::cerr << "Unrecognised VME module ID (" << std::dec;
			std::cerr << mbs_sevt->GetModuleID() << "), with sub-event type 0x";
			std::cerr << std::hex << mbs_sevt->GetSubEventType() << " (";
			std::cerr << mbs_sevt->GetSubEventDescription() << ")";
			std::cerr << std::dec << std::endl;
			continue;

		}

	}

	// Suppress unused warnings
	(void)nblock;

	return;

}

//-----------------------------------------------------------------------------
// Treat a Mesytec ADC data item
void MiniballMedConverter::ProcessMesytecAdcData() {

	// Loop over all the available data
	unsigned int i = 0;
	while( i < mbs_sevt->GetNumberOfData() ){

		// Header of the sub event (module number in first word)
		unsigned short header = mbs_sevt->GetData(i++);
		unsigned short mod = header & MESYTEC_MADC_MODULE_ID;

		// Convert to logical module from Marabou numbering
		if( mod >= set->GetNumberOfAdcModules() )
			mod = set->GetMesytecAdcModuleNumber( mod );
		else mod--; // Mesytec modules count from 1 in Marabou!!

		// Get second word of the header and test format
		header = mbs_sevt->GetData(i++);
		if( header & MESYTEC_MADC_OUTPUT_FORMAT ) {

			std::cerr << __PRETTY_FUNCTION__ << ": Error, output ";
			std::cerr << "format (highest bit) should be zero (header = ";
			std::cerr << std::hex << header << std::dec << ")" << std::endl;
			return;

		}

		// If format is fine, we have the word count in here
		int wc = (header & MESYTEC_MADC_WORD_COUNT);

		// check number of Channels (MESYTEC_MADC_NBOFCHAN=32 + 1 word End of Event + 1 extended timestamp)
		// -> skip TOTAL subevent if wrong number of Channels
		if( wc <= 0 || wc > (int)set->GetNumberOfMesytecAdcChannels() + 2 ) {

			std::cout << __PRETTY_FUNCTION__ << ": read event nr. ";
			std::cout << my_event_id << ": wrong Word Count: ";
			std::cout << wc << " -> skip TOTAL subevent" << std::endl;
			return;

		}

		// Loop over number of channels for which data follows (maybe also extended timestamp)
		// Might not be all channel, so use wc, but wc includes the end of event
		std::vector<unsigned short> ch_vec;
		std::vector<unsigned short> qshort_vec;
		long long Timestamp = 0;
		bool clipped = false;
		for( unsigned short ch = 0; ch < wc - 1; ch++ ) {

			// Check if type of word (highest two bits) is end of event
			unsigned short test = mbs_sevt->GetData(i++);

			// 00 => data, 11 => end of event
			if( ( test & MESYTEC_MADC_END_OF_EVENT ) == MESYTEC_MADC_END_OF_EVENT ) {

				std::cerr << "Error, found end of event data (" << test << ") after " << ch+1;
				std::cerr << " words but should have a word count of " << wc << std::endl;
				return;

			}

			// What if only one of the higest two bits is non-zero? weird
			else if( test & MESYTEC_MADC_END_OF_EVENT )	{

				std::cerr << "Error, found weird data (" << test << ") after ";
				std::cerr << ch+1 << " words with word count of " << wc << std::endl;
				return;

			}

			// Check whether this is the extended timestamp
			if( test & MESYTEC_MADC_EXTENDED_TIMESTAMP ) {

				Timestamp = ((long long)mbs_sevt->GetData(i++)) << MESYTEC_MADC_EXTENDED_TIMESTAMP_SHIFT;
				//Timestamp &= MESYTEC_MADC_EXTENDED_TIMESTAMP_MASK;
				continue;

			}

			// If we don't have the timestamp here, test has the channel number
			// Get actual Channel number (bits[21...16] of data word)
			unsigned short chanNo = (test & MESYTEC_MADC_CHANNEL_NUMBER);
			ch_vec.push_back( chanNo );

			// But the energy is in the next word, lowest 12 bits
			unsigned short Qshort = (mbs_sevt->GetData(i++) & MESYTEC_MADC_VALUE);
			qshort_vec.push_back( Qshort );

			// Check the out of range bit
			if( mbs_sevt->GetData(i) & MESYTEC_MADC_OUT_OF_RANGE )
				clipped = true;
			else clipped = false;

		} // loop over wc - 1

		// Trailer of the sub event (module number in first word)
		unsigned int trailer = ((unsigned int)mbs_sevt->GetData(i++) << 16) & 0xffff0000;
		trailer |= ((unsigned int)mbs_sevt->GetData(i++)) & 0x0000ffff;

		// Check type of word (highest two bits should be set)
		if ( ( trailer & MESYTEC_MADC_END_OF_EVENT ) != MESYTEC_MADC_END_OF_EVENT ) {

			std::cout << __PRETTY_FUNCTION__ << ": read event nr. " << my_event_id;
			std::cout << ": wrong EOE word of type: " << ( trailer & MESYTEC_MADC_END_OF_EVENT );
			std::cout << " -> skip TOTAL subevent" << std::endl;
			return;

		}

		// Merge the full time stamp and add the DGF delay
		Timestamp |= ( trailer & MESYTEC_MADC_TIMESTAMP );
		Timestamp += set->GetDgfTimestampDelay();
		Timestamp *= set->GetMesytecAdcTimestampUnits();

		// Now we have the data, fill the tree
		for( unsigned item = 0; item < qshort_vec.size(); item++ ){

			// Clear the old stuff
			adc_data->ClearData();
			//data_packet->ClearData();

			// Some basic info for every event
			adc_data->SetEventID( my_event_id );

			// Calculate energy and threshold
			float energy = cal->AdcEnergy( mod, ch_vec[item], qshort_vec[item] );
			bool thresh = cal->AdcThreshold( mod, ch_vec[item] );

			// Corrected time for ADCs
			long long time_corr = Timestamp;
			time_corr += cal->AdcTime( mod, ch_vec[item] );

			// Set values for data item
			adc_data->SetTime( time_corr ); // only works for MADC, CAEN needs reconstruction
			adc_data->SetQshort( qshort_vec[item] );
			adc_data->SetModule( mod );
			adc_data->SetChannel( (char)ch_vec[item] );
			adc_data->SetEnergy( energy );
			adc_data->SetThreshold( thresh );
			adc_data->SetClipped( clipped );

			// Fill the tree
			if( !flag_source ) {
				std::shared_ptr<MiniballDataPackets> data_packet =
					std::make_shared<MiniballDataPackets>( adc_data );
				data_vector.emplace_back( data_packet );
				data_map.push_back( std::make_pair<unsigned long,double>(
					data_vector.size()-1, data_packet->GetTime() ) );
			}

			// Fill histograms
			hadc_qshort[mod][ch_vec[item]]->Fill( qshort_vec[item] );
			hadc_cal[mod][ch_vec[item]]->Fill( energy );

		}

	}

	return;

}

//-----------------------------------------------------------------------------
// Treat a CAEN ADC data item
void MiniballMedConverter::ProcessCaenAdcData() {

	// Do nothing with it for now, because I'm lazy
	return;

}

//-----------------------------------------------------------------------------
// Decode the pattern unit data and add it to the packets
void MiniballMedConverter::ProcessPatternUnitData() {

	// Test data comes from scaler unit?
	if( mbs_sevt->GetSubEventType() != MBS_STYPE_VME_SIS_3 ){

		std::cerr << __PRETTY_FUNCTION__;
		std::cerr << ": Error - got data from VME module id ";
		std::cerr << mbs_sevt->GetModuleID();
		std::cerr << ", which is defined as a pattern unit but has the";
		std::cerr << " wrong data format: " << std::hex;
		std::cerr << mbs_sevt->GetSubEventType() << std::dec << std::endl;
		return;

	}

	//mbs_sevt->Show();

	// Loop over data and reconstruct header and data
	unsigned int i = 0;
	while( i < mbs_sevt->GetNumberOfData() ) {

		// Get the header
		unsigned short header = mbs_sevt->GetData(i++);
		if( ( header & SIS3600_D_HDR ) == 0 ) {

			std::cout << __PRETTY_FUNCTION__ << ": read event nr. ";
			std::cout << my_event_id << ", sub event nr. ";
			std::cout << (int)mbs_sevt->GetSubEventID();
			std::cout << ": wrong Header word of type 0x" << std::hex;
			std::cout << header << " -> skip subevent" << std::dec << std::endl;
			return;

		}

		// Get the module number
		short mod = set->GetPatternUnitNumber( header & SIS3600_MSERIAL );
		if( mod < 0 ) return;

		// Next item is the word count
		unsigned short wc = mbs_sevt->GetData(i++);

		// Loop over data and reconstruct integers
		for( unsigned short j = 0; j < wc-2; j++ ) {

			unsigned int data;
			data  = ( (unsigned int)mbs_sevt->GetData(i++) ) << 16;
			data |= ( (unsigned int)mbs_sevt->GetData(i++) );

			if( data > 0 )
				mbsinfo_packet->AddPattern( mod, j, data );

		}

	}

	return;

}

//-----------------------------------------------------------------------------
// Decode the scaler unit data and add it to the packets
void MiniballMedConverter::ProcessScalerData() {

	// Test data comes from scaler unit?
	if( mbs_sevt->GetSubEventType() != MBS_STYPE_CAMAC_WO_ID_1 ){

		std::cerr << __PRETTY_FUNCTION__;
		std::cerr << ": Error - got data from VME module id ";
		std::cerr << mbs_sevt->GetModuleID();
		std::cerr << ", which is defined as a scaler unit but has the";
		std::cerr << " wrong data format: " << std::hex;
		std::cerr << mbs_sevt->GetSubEventType() << std::dec << std::endl;
		return;

	}

	// Check we have an even number of data
	if( mbs_sevt->GetNumberOfData() % 2 != 0 ){

		std::cerr << __PRETTY_FUNCTION__;
		std::cerr << ": Error - expecting even number of words in MbsSubEvent but there are ";
		std::cerr << mbs_sevt->GetNumberOfData() << std::endl;
		return;

	}

	// Loop over data and reconstruct integers
	for( unsigned int i = 0; i < mbs_sevt->GetNumberOfData()/2; i++ ) {

		unsigned int data;
		data  = ( (unsigned int)mbs_sevt->GetData(i+0) ) << 16;
		data |= ( (unsigned int)mbs_sevt->GetData(i+1) );

		if( data > 0 )
			mbsinfo_packet->AddScaler( i, data );

	}

	return;

}

//-----------------------------------------------------------------------------
// Treat a DGF scaler data item
void MiniballMedConverter::ProcessDgfScaler() {

	// Test data comes from DGFscaler unit?
	if( !set->CheckVmeModuleIsDgfScaler( mbs_sevt->GetModuleID() ) ){

		std::cerr << __PRETTY_FUNCTION__;
		std::cerr << ": Error - got data from VME module id ";
		std::cerr << mbs_sevt->GetModuleID();
		std::cerr << ", which looks like a DGF scaler module but it isn't";
		std::cerr << " defined as such in settings file" << std::endl;
		return;

	}

	// Loop over all the available data
	unsigned int i = 0;
	while( i < mbs_sevt->GetNumberOfData() ){

		// Finish the data?
		if( mbs_sevt->GetData(i) == DGF_SCALER_END_OF_BUFFER )
			break;

		// Check for magic word at the start
		if( mbs_sevt->GetData(i++) != DGF_SCALER_MAGIC_WORD ){

			std::cerr << "Internal dgf scalers: data out of phase - 0x";
			std::cerr << std::hex << mbs_sevt->GetData(--i);
			std::cerr << " (should be magic word 0x" << DGF_SCALER_MAGIC_WORD ;
			std::cerr << ")" << std::dec << std::endl;
			return;

		}

		// Get the header data
		int wc  = mbs_sevt->GetData(i++);	// word count
		int clu = mbs_sevt->GetData(i++);	// cluster id
		int mod = mbs_sevt->GetData(i++); 	// module id

		// Break if we have nonsense
		if( clu < 0 || mod < 0 ) return;
		if( wc < 0 ){
			std::cerr << "Word count negative: " << wc << std::endl;
			return;
		}
		if( (int)mbs_sevt->GetNumberOfData() - (int)(i-2) < DGF_SCALER_MIN_SIZE ){
			std::cerr << "Not enough data left in sub event for new DGF scaler: ";
			std::cerr << mbs_sevt->GetNumberOfData() - (i-2) << " < minsize(";
			std::cerr << DGF_SCALER_MIN_SIZE << ")" << std::endl;
			return;
		}
		if( (int)mbs_sevt->GetNumberOfData() - (int)(i-2) < wc ){
			std::cerr << "Not enough data left in sub event for new DGF scaler: ";
			std::cerr << mbs_sevt->GetNumberOfData() - (i-2) << " < wc(" << wc << ")" << std::endl;
			return;
		}

		// Get the real time data
		unsigned int index = i + DGF_SCALER_INDEX_REALTIME;
		long long realTime = mbs_sevt->GetData(index);
		realTime |= ( (unsigned long long)mbs_sevt->GetData(index+1) ) << 16;
		realTime |= ( (unsigned long long)mbs_sevt->GetData(index+2) ) << 32;

		// Get the run time data
		index = i + DGF_SCALER_INDEX_RUNTIME;
		long long runTime = mbs_sevt->GetData(index);
		runTime |= ( (unsigned long long)mbs_sevt->GetData(index+1) ) << 16;
		runTime |= ( (unsigned long long)mbs_sevt->GetData(index+2) ) << 32;

		// Get the GSLT time data
		index = i + DGF_SCALER_INDEX_GSLTTIME;
		long long gsltTime = mbs_sevt->GetData(index);
		gsltTime |= ( (unsigned long long)mbs_sevt->GetData(index+1) ) << 16;
		gsltTime |= ( (unsigned long long)mbs_sevt->GetData(index+2) ) << 32;

		// Get the number of events
		index = i + DGF_SCALER_INDEX_NEVENTS;
		long long nEvents = mbs_sevt->GetData(index);
		nEvents |= ( (unsigned long long)mbs_sevt->GetData(index+1) ) << 16;
		nEvents |= ( (unsigned long long)mbs_sevt->GetData(index+2) ) << 32;

		// Make a DGF scaler event and add it to the packets
		DgfScalerData s( set->GetNumberOfDgfChannels() );
		s.SetModule( mod );
		s.SetClusterID( clu );
		s.SetRealTime( realTime );
		s.SetRunTime( runTime );
		s.SetGSLTTime( gsltTime );

		// Get channel by channel data
		for( unsigned int j = 0; j < set->GetNumberOfDgfChannels(); j++ ){

			index  = i + DGF_SCALER_INDEX_CH_OFFSET;
			index += j * DGF_SCALER_INDEX_CH_SIZE;

			// Get LiveTime for this channel
			long long liveTime = mbs_sevt->GetData(index);
			liveTime |= ( (unsigned long long)mbs_sevt->GetData(index+1) ) << 16;
			liveTime |= ( (unsigned long long)mbs_sevt->GetData(index+2) ) << 32;

			// Get FastPeak for this channel
			int fastPeak = mbs_sevt->GetData(index+3);
			fastPeak |= ( (unsigned long long)mbs_sevt->GetData(index+4) ) << 16;

			s.SetLiveTime( j, liveTime );
			s.SetFastPeak( j, fastPeak );

		}

		// Move forward to next data item
		i += wc - 4; // 4 header words included in word count

	}

	return;

}


//-----------------------------------------------------------------------------
// Treat a DGF data item
void MiniballMedConverter::ProcessDgfData() {

	// Test data comes from scaler unit?
	if( mbs_sevt->GetSubEventType() != MBS_STYPE_CAMAC_DGF_3 ){

		std::cerr << __PRETTY_FUNCTION__;
		std::cerr << ": Error - got data from VME module id ";
		std::cerr << mbs_sevt->GetModuleID();
		std::cerr << ", which looks like a DGF module but it isn't";
		std::cerr << " defined as such in settings file" << std::endl;
		return;

	}

	// Loop over all the available data
	int i = 0;
	int wc = mbs_sevt->GetNumberOfData();
	while( i < wc ){

		//std::cout << std::dec << "Readout nr.: " << ev->GetEventID()-2586617;
		//std::cout << ", wc = " << wc << ", length = " << mbs_sevt->GetDataLength();
		//std::cout << std::hex << ", DGF data(0) = 0x" << mbs_sevt->GetData(0) << std::endl;

		// Header of the sub event
		unsigned short start = i;
		unsigned short length = mbs_sevt->GetData(i++);
		unsigned short end = start + length;
		unsigned short mod = mbs_sevt->GetData(i++);

		// Check length
		if( end > mbs_sevt->GetDataLength() ){

			std::cout << __PRETTY_FUNCTION__ << ": XIA wrong buffer length: ";
			std::cout << std::dec << length << " vs. " << mbs_sevt->GetDataLength() * sizeof(unsigned short);
			std::cout << ", Start: " << start;
			std::cout << ", End: " << end << std::endl;
			return; // skip total subevent

		}

		// Get buffer format descriptor (=RUNTASK)
		unsigned short format = mbs_sevt->GetData(i++);

		// Parameters for holding the time
		unsigned short RunTimeA, RunTimeB, RunTimeC;
		unsigned int BufferTime;
		unsigned long long RunTime;

		// Check if known buffer format
		if( ( format != STD_LM_BUFFORMAT )   &&
		   ( format != COMP_LM_BUFFORMAT )  &&
		   ( format != COMP_FLM_BUFFORMAT ) &&
		   ( format != COMP3_LM_BUFFORMAT ) &&
		   ( format != STD_FLM_BUFFORMAT ) ) {

			std::cout << __PRETTY_FUNCTION__ << ": read out event ";
			std::cout << my_event_id << ": wrong buffer format: ";
			std::cout << format << " !!!" << std::endl;
			return; // skip total subevent

		}

		else {

			// read words 3-5 of buffer header: 3.: high-, 4.: middle-, 5.: low-word of run start time
			RunTimeA = mbs_sevt->GetData(i++);      // high
			RunTimeB = mbs_sevt->GetData(i++);      // mid
			RunTimeC = mbs_sevt->GetData(i++);      // low

			// set 'buffertime' and 'runtime'
			RunTime     = ( (unsigned long long)RunTimeA << 32) & 0xffff000000000000;
			BufferTime  = ( (unsigned int)RunTimeB << 16) & 0xffff0000;
			BufferTime |= ( (unsigned int)RunTimeC ) & 0x0000ffff;
			RunTime    |= ( BufferTime & 0xffffffff);

			// Update timestamp using a DGF module
			if( my_good_tm_stp == 0 )
				my_good_tm_stp = RunTime * set->GetDgfTimestampUnits();
			//my_good_tm_stp = RunTime;

		}

		// from here on: always check buffer format (=RUNTASK) before filling 'dgf' or 'dgf_rt259'
		// beam dump module has format 'COMP3_LM_BUFFORMAT'
		// determine corrected module number
		mod = set->GetDgfModuleNumber( mod );

		// Work out what type of DGF event we have
		int DgfType = mbs_sevt->GetSubEventType() - XIA_EVENT;
		(void)DgfType; // not yet used anywhere

		// Get data for this module
		while( i < end ){

			// Read 3 words of event header
			// 1.: hitpattern, 2.: high-, 3.: low-word of event time
			unsigned short HitPattern    = mbs_sevt->GetData(i++);
			unsigned short EventTimeHigh = mbs_sevt->GetData(i++);
			unsigned short EventTimeLow  = mbs_sevt->GetData(i++);

			// Set 'eventtime'
			long long EventTime = (( (long long)EventTimeHigh ) << 16) & 0xffff0000;
			EventTime |= (long long)EventTimeLow & 0x0000ffff;

			// Check for overflow and build full event time
			if( EventTime <= BufferTime ) RunTimeA++;
			EventTime |= (( (long long)RunTimeA ) << 32) & 0xffff00000000;


			// check hitpattern: at least one channel bit has to be set
			// for all but TS_EBIS_T1_T2_MODULE and 4 CD TS-modules
			// _all_ timestamp modules with RUNTASK!=259 -> 'ModuleNumber'
			// can be directly compared with 'analysis module number'
			if( !( HitPattern & 0xf ) && !set->IsTimestampModule(mod) ) {

				std::cout << __PRETTY_FUNCTION__ << ": XIA hitpattern error: hitpattern = ";
				std::cout << HitPattern << mod << ": module: " << mod;
				std::cout << ", i: " << i << ", end: " << end;
				std::cout << ", Data in SubEvent: " << mbs_sevt->GetNumberOfData();
				std::cout << std::endl;

				// skip module, but not full sub event
				i = end;
				break;

			}

			else {

				// Process hit for all channels
				for( unsigned int ch = 0; ch < set->GetNumberOfDgfChannels(); ch++ ) {

					// Channel mask to make sure it's got data
					unsigned int ChannelMask = 1 << ch;
					if( ( HitPattern & ChannelMask ) != 0 ) {

						//std::cout << std::hex << HitPattern << " & ";
						//std::cout << ChannelMask << " = " << ( HitPattern & ChannelMask );
						//std::cout << std::dec << std::endl;

						// Now different data following for RUNTASK=259 resp. others
						// RUNTASK!=259: now #of words for this channel
						unsigned short ChannelLength;
						if( format != COMP3_LM_BUFFORMAT )
							ChannelLength = mbs_sevt->GetData(i++);

						// Next 2 words: fast trigger time & energy for ALL diff. RUNTASKs
						unsigned short FastTriggerTime = mbs_sevt->GetData(i++);
						unsigned short Qshort = mbs_sevt->GetData(i++);

						// Sort out long fast trigger time and wrap around
						unsigned long long LongFastTriggerTime = FastTriggerTime;
						if( FastTriggerTime > EventTimeLow )
							LongFastTriggerTime += 65536ll*EventTimeHigh + 65536ll*65536ll*RunTimeA;
						else
							LongFastTriggerTime += 65536ll*EventTimeHigh + 65536ll + 65536ll*65536ll*RunTimeA;

						// Get calibrated energy and check threshold
						float energy = cal->DgfEnergy( mod, ch, Qshort );
						bool thresh = cal->DgfThreshold( mod, ch );

						// For RUNTASK!=259, now 6 user PSA values (& possible trace) follow
						std::vector<unsigned short> UserValues;
						std::vector<unsigned short> trace;
						if( format != COMP3_LM_BUFFORMAT ) {

							// Get 6 user values for PSA
							for( unsigned char j = 0; j < 6; j++ )
								UserValues.push_back( mbs_sevt->GetData(i++) );

							// Read out trace
							unsigned int TraceLength = (int)ChannelLength - CHANHEADLEN;
							for( unsigned char j = 0; j < TraceLength; j++ )
								trace.push_back( mbs_sevt->GetData(i++) );

						} // check format for PSA and trace

						//-----------
						// Now let's add the data
						//------------

						// Fill histograms
						hdgf_qshort[mod][ch]->Fill( Qshort );
						hdgf_cal[mod][ch]->Fill( energy );

						// Clear the old stuff
						dgf_data->ClearData();
						info_data->ClearData();
						//data_packet->ClearData();

						//std::cout << "DGF module " << mod << ", time = ";
						//std::cout << LongFastTriggerTime << ", Qshort = ";
						//std::cout << Qshort << std::endl;

						long long time_corr = LongFastTriggerTime * set->GetDgfTimestampUnits();
						time_corr += cal->DgfTime( mod, ch );

						// Check if it's a timestamper!
						if( set->IsTimestampModule( mod ) ) {

							// Check if it matches EBIS, T1 or SC
							char mycode = -1;
							if( mod == set->GetEBISDgf() && ch == set->GetEBISChannel() )
								mycode = set->GetEBISCode();
							else if( mod == set->GetT1Dgf() && ch == set->GetT1Channel() )
								mycode = set->GetT1Code();
							else if( mod == set->GetSCDgf() && ch == set->GetSCChannel() )
								mycode = set->GetSCCode();

							// Useful only if it matched
							if( mycode >= 0 ) {

								// Set values for data item
								info_data->SetEventID( my_event_id );
								info_data->SetTime( time_corr );
								info_data->SetCode( mycode );
								info_data->SetBoard( mod );

								// Fill the tree
								if( !flag_source ) {
									std::shared_ptr<MiniballDataPackets> data_packet =
										std::make_shared<MiniballDataPackets>( info_data );
									data_vector.emplace_back( data_packet );
									data_map.push_back( std::make_pair<unsigned long,double>(
										data_vector.size()-1, data_packet->GetTime() ) );
								}

							}

						}

						else {

							// Set values for data item
							dgf_data->SetEventID( my_event_id );
							dgf_data->SetRunTime( RunTime * set->GetDgfTimestampUnits() );
							dgf_data->SetEventTime( EventTime * set->GetDgfTimestampUnits() );
							dgf_data->SetFastTriggerTime( FastTriggerTime * set->GetDgfTimestampUnits() );
							dgf_data->SetLongFastTriggerTime( time_corr );
							dgf_data->SetHitPattern( HitPattern );
							dgf_data->SetQshort( Qshort );
							dgf_data->SetModule( mod );
							dgf_data->SetChannel( ch );
							dgf_data->SetEnergy( energy );
							dgf_data->SetThreshold( thresh );
							dgf_data->SetUserValues( UserValues );
							dgf_data->SetUserValues( trace );

							// Fill the tree
							if( !flag_source ) {
								std::shared_ptr<MiniballDataPackets> data_packet =
									std::make_shared<MiniballDataPackets>( dgf_data );
								data_vector.emplace_back( data_packet );
								data_map.push_back( std::make_pair<unsigned long,double>(
									data_vector.size()-1, data_packet->GetTime() ) );
							}

						}

					} // channel mask, we have data in this channel

				} // loop over channels

			} // else of 'if(!(HitPattern&0xf) && ModuleNumber!=TS_EBIS_T1_T2_MODULE...)'

		} // i < mod: data for this module

		// MBS data: always 32bit words -> if len odd -> fill word has to be skipped
		//std::cout << std::dec << "length = " << length << ", wc = " << wc << ", i = " << i << std::endl;
		if( length & 1 ) i++;

	} // data in this sub event

	return;

}


// Function to run the conversion for a single file
int MiniballMedConverter::ConvertFile( std::string input_file_name,
									  unsigned long start_subevt,
									  long end_subevt ) {

	// Uncomment to force only a few subevts - debug
	//end_subevt = 1000;

	// Read the file.
	std::ifstream input_file( input_file_name, std::ios::in|std::ios::binary );
	if( !input_file.is_open() ){

		std::cout << "Cannot open " << input_file_name << std::endl;
		return -1;

	}

	// Reset counters to zero for every file
	StartFile();

	// Calculate the size of the file.
	input_file.seekg( 0, input_file.end );
	unsigned long long size_end = input_file.tellg();
	input_file.seekg( 0, input_file.beg );
	unsigned long long size_beg = input_file.tellg();
	unsigned long long FILE_SIZE = size_end - size_beg;

	// Calculate the number of blocks in the file.
	unsigned long BLOCKS_NUM = FILE_SIZE / set->GetBlockSize();

	std::cout << "Opening file: " << input_file_name << std::endl;
	sslogs << "File size = " << (double)FILE_SIZE/1024./1024.;
	sslogs << " MB" << std::endl;
	std::cout << sslogs.str() << std::endl;
	sslogs.str( std::string() ); // clean up

	// Close the file
	input_file.close();

	// Create an MED data instance and set block/buffer size etc
	MBS mbs;
	mbs.SetBufferSize( set->GetBlockSize() );
	mbs.OpenMedFile( input_file_name );

	// Loop over all the MBS Events
	unsigned long mbsevt = 0, nblock = 0;
	for( mbsevt = 0; ; mbsevt++ ){

		// Calculate how many blocks we have used and progress
		nblock = mbs.GetBufferCount();
		if( nblock % 200 == 0 || nblock+1 == BLOCKS_NUM ) {

			// Percent complete
			float percent = (float)(nblock+1)*100.0/(float)BLOCKS_NUM;

			// Progress bar in GUI
			if( _prog_ ){

				prog->SetPosition( percent );
				gSystem->ProcessEvents();

			}

			// Progress bar in terminal
			std::cout << " " << std::setw(8) << std::setprecision(4);
			std::cout << percent << "%\r";
			std::cout.flush();

		}

		// Get the next event - returns nullptr at the end of the file and sets eof to true
		ev = mbs.GetNextMedEvent();
		if( mbs.IsEof() ) break;
		if( !ev ) continue;
		my_event_id = ev->GetEventID();

		// Don't bother if we're not running
		if( !mbs.IsRunning() ) continue;

		if( my_event_id == 0 )
			std::cout << "Bad event ID in data" << std::endl;


		// Check if we are before the start sub event or after the end sub events
		if( mbsevt < start_subevt || ( (long)mbsevt > end_subevt && end_subevt > 0 ) )
			continue;

		// Process current block
		mbsinfo_packet->ClearData();
		ProcessEvent( mbsevt );

		// Write the MBS event info
		mbsinfo_packet->SetTime( my_good_tm_stp );
		mbsinfo_packet->SetEventID( my_event_id );
		mbsinfo_tree->Fill();

	} // loop - mbsevt < MBS_EVENTS

	// Close the file
	mbs.CloseFile();

	// Print stats
	std::cout << std::endl;

	return mbsevt;

}
