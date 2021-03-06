//
// Copyright 2014 Per Vices Corporation
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <uhd/property_tree.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/usrp/multi_crimson_tng.hpp>
#include <uhd/utils/msg.hpp>
#include <uhd/exception.hpp>
#include <uhd/utils/log.hpp>
#include <uhd/utils/math.hpp>
#include <uhd/utils/gain_group.hpp>
#include <uhd/usrp/dboard_id.hpp>
#include <uhd/usrp/mboard_eeprom.hpp>
#include <uhd/usrp/dboard_eeprom.hpp>
#include <uhd/convert.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <cmath>

#define CRIMSON_MASTER_CLOCK_RATE	322265625
#define CRIMSON_RX_CHANNELS 4
#define CRIMSON_TX_CHANNELS 4

using namespace uhd;
using namespace uhd::usrp;

/***********************************************************************
 * Helper Functions
 **********************************************************************/
// TODO Need to check if: the sample rate, channels enabled, and bytes per I/Q sample,
// will overflow, or under flow the data stream
bool _check_tng_link_rate(const stream_args_t &args) {
	// insert logic to calculate bytes_per_sample

	// insert logic to compare link_max_rate > 0 && sample_rate * bytes_per_sample
	// <> link_max_rate

	// assume no under/overflow
	return true;
}



/***********************************************************************
 * Multi Crimson Implementation
 **********************************************************************/
multi_crimson_tng::multi_crimson_tng(const device_addr_t &addr) {
    // this make will invoke the correct inherited crimson device class
    _dev  = device::make(addr, device::CRIMSON_TNG);
    _tree = _dev  -> get_tree();
}

multi_crimson_tng::~multi_crimson_tng(void) {
    // do nothing
}

device::sptr multi_crimson_tng::get_device(void){
    return _dev;
}

// ID = unique ID set for the device
// NAME = Name of the board (digital board will be crimson)
// SERIAL = manufacturer serial no.
// FW VERSION = verilog version, if NA, will be same as SW version
// HW VERSION = PCB version
// SW VERSION = software version

dict<std::string, std::string> multi_crimson_tng::get_usrp_rx_info(size_t chan) {
    dict<std::string, std::string> crimson_info;

    crimson_info["mboard_id"]       = _tree->access<std::string>(mb_root(0) / "id").get();
    crimson_info["mboard_name"]     = _tree->access<std::string>(mb_root(0) / "name").get();
    crimson_info["mboard_serial"]   = _tree->access<std::string>(mb_root(0) / "serial").get();

    crimson_info["clock_id"]        = _tree->access<std::string>(mb_root(0) / "time" / "id").get();
    crimson_info["clock_name"]      = _tree->access<std::string>(mb_root(0) / "time" / "name").get();
    crimson_info["clock_serial"]    = _tree->access<std::string>(mb_root(0) / "time" / "serial").get();

    crimson_info["rx_id"]           = _tree->access<std::string>(mb_root(0) / "rx" / "id").get();
    crimson_info["rx_subdev_name"]  = _tree->access<std::string>(mb_root(0) / "rx" / "name").get();
    crimson_info["rx_subdev_spec"]  = _tree->access<std::string>(mb_root(0) / "rx" / "spec").get();
    crimson_info["rx_serial"]       = _tree->access<std::string>(mb_root(0) / "rx" / "serial").get();
    //crimson_info["rx_antenna"]      = _tree->access<std::string>(mb_root(0) / "rx" / "antenna").get();

    return crimson_info;
}

dict<std::string, std::string> multi_crimson_tng::get_usrp_tx_info(size_t chan) {
    dict<std::string, std::string> crimson_info;

    crimson_info["mboard_id"]       = _tree->access<std::string>(mb_root(0) / "id").get();
    crimson_info["mboard_name"]     = _tree->access<std::string>(mb_root(0) / "name").get();
    crimson_info["mboard_serial"]   = _tree->access<std::string>(mb_root(0) / "serial").get();

    crimson_info["clock_id"]        = _tree->access<std::string>(mb_root(0) / "time" / "id").get();
    crimson_info["clock_name"]      = _tree->access<std::string>(mb_root(0) / "time" / "name").get();
    crimson_info["clock_serial"]    = _tree->access<std::string>(mb_root(0) / "time" / "serial").get();

    crimson_info["tx_id"]           = _tree->access<std::string>(mb_root(0) / "tx" / "id").get();
    crimson_info["tx_subdev_name"]  = _tree->access<std::string>(mb_root(0) / "tx" / "name").get();
    crimson_info["tx_subdev_spec"]  = _tree->access<std::string>(mb_root(0) / "tx" / "spec").get();
    crimson_info["tx_serial"]       = _tree->access<std::string>(mb_root(0) / "tx" / "serial").get();
    //crimson_info["tx_antenna"]      = _tree->access<std::string>(mb_root(0) / "tx" / "antenna").get();

    return crimson_info;
}

/*******************************************************************
 * Mboard methods
 ******************************************************************/
// Master clock rate is fixed at 322.265625 MHz
void multi_crimson_tng::set_master_clock_rate(double rate, size_t mboard){
    return;
}

double multi_crimson_tng::get_master_clock_rate(size_t mboard){
    return _tree->access<double>(mb_root(0) / "tick_rate").get();
}

std::string multi_crimson_tng::get_pp_string(void){
    std::string buff = str(boost::format(
        "%s Crimson:\n"
        "  Device: %s\n"
    )
        % ((get_num_mboards() > 1)? "Multi" : "Single")
        % (_tree->access<std::string>("/name").get())
    );
    for (size_t m = 0; m < get_num_mboards(); m++){
        buff += str(boost::format(
            "  Mboard %d: %s\n"
        ) % m
            % (_tree->access<std::string>(mb_root(m) / "name").get())
        );
    }

    //----------- rx side of life ----------------------------------
    for (size_t m = 0, chan = 0; m < get_num_mboards(); m++){
        for (; chan < (m + 1)*get_rx_subdev_spec(m).size(); chan++){
            buff += str(boost::format(
                "  RX Channel: %u\n"
                "    RX DSP: %s\n"
                "    RX Dboard: %s\n"
                "    RX Subdev: %s\n"
            ) % chan
                % rx_dsp_root(chan).leaf()
                % rx_rf_fe_root(chan).branch_path().branch_path().leaf()
                % (_tree->access<std::string>(rx_rf_fe_root(chan) / "name").get())
            );
        }
    }

    //----------- tx side of life ----------------------------------
    for (size_t m = 0, chan = 0; m < get_num_mboards(); m++){
        for (; chan < (m + 1)*get_tx_subdev_spec(m).size(); chan++){
            buff += str(boost::format(
                "  TX Channel: %u\n"
                "    TX DSP: %s\n"
                "    TX Dboard: %s\n"
                "    TX Subdev: %s\n"
            ) % chan
                % tx_dsp_root(chan).leaf()
                % tx_rf_fe_root(chan).branch_path().branch_path().leaf()
                % (_tree->access<std::string>(tx_rf_fe_root(chan) / "name").get())
            );
        }
    }
    return buff;
}

// Get the Digital board name
std::string multi_crimson_tng::get_mboard_name(size_t mboard){
    return _tree->access<std::string>(mb_root(0) / "name").get();
}

// Get the current time on Crimson
time_spec_t multi_crimson_tng::get_time_now(size_t mboard){
    return _tree->access<time_spec_t>(mb_root(0) / "time/now").get();
}

// Get the time of the last PPS (pulse per second)
time_spec_t multi_crimson_tng::get_time_last_pps(size_t mboard){
    return _tree->access<time_spec_t>(mb_root(0) / "time/pps").get();
}

// Set the current time on Crimson
void multi_crimson_tng::set_time_now(const time_spec_t &time_spec, size_t mboard){
    _tree->access<time_spec_t>(mb_root(0) / "time/now").set(time_spec);
    return;
}

// Set the time for the next PPS (pulse per second)
void multi_crimson_tng::set_time_next_pps(const time_spec_t &time_spec, size_t mboard){
    _tree->access<time_spec_t>(mb_root(0) / "time/pps").set(time_spec);
    return;
}

void multi_crimson_tng::set_time_unknown_pps(const time_spec_t &time_spec){
    // Not implemented
    throw uhd::not_implemented_error("timed command feature not implemented on this hardware");
}

bool multi_crimson_tng::get_time_synchronized(void){
    // Not implemented
    throw uhd::not_implemented_error("timed command feature not implemented on this hardware");
    return true;
}
void multi_crimson_tng::set_command_time(const time_spec_t &time_spec, size_t mboard){
    // Not implemented
    throw uhd::not_implemented_error("timed command feature not implemented on this hardware");
    return;
}
void multi_crimson_tng::clear_command_time(size_t mboard){
    // Not implemented
    throw uhd::not_implemented_error("timed command feature not implemented on this hardware");
    return;
}

void multi_crimson_tng::issue_stream_cmd(const stream_cmd_t &stream_cmd, size_t chan){
    // set register to start the stream
    if( stream_cmd.stream_mode == stream_cmd_t::STREAM_MODE_START_CONTINUOUS) {
        _tree->access<std::string>(rx_link_root(chan) / "stream").set("1");

    // set register to stop the stream
    } else if (stream_cmd.stream_mode == stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS) {
        _tree->access<std::string>(rx_link_root(chan) / "stream").set("0");

    } else if (stream_cmd.stream_mode == stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE) {
	// set register to wait for a stream cmd after num_samps
	// not supported in Crimson

    } else if (stream_cmd.stream_mode == stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_MORE) {
	// set register to not wait for a stream cmd after num_samps
	// not supported in Crimson

    } else {
        _tree->access<std::string>(rx_link_root(chan) / "stream").set("0");
    }
    return;
}

void multi_crimson_tng::set_clock_config(const clock_config_t &clock_config, size_t mboard) {
    //set the reference source...
    std::string clock_source;
    switch(clock_config.ref_source){
    case clock_config_t::REF_INT: clock_source = "internal"; break;
    case clock_config_t::REF_SMA: clock_source = "external"; break;
    default: clock_source = "internal";
    }
    _tree->access<std::string>(mb_root(0) / "clock_source" / "value").set(clock_source);

    //set the time source
    std::string time_source;
    switch(clock_config.pps_source){
    case clock_config_t::PPS_INT: time_source = "internal"; break;
    case clock_config_t::PPS_SMA: time_source = "external"; break;
    default: time_source = "internal";
    }
    _tree->access<std::string>(mb_root(0) / "time_source" / "value").set(clock_source);

    return;
}

// set the current time source
void multi_crimson_tng::set_time_source(const std::string &source, const size_t mboard){
    _tree->access<std::string>(mb_root(0) / "time_source" / "value").set(source);
    return;
}

// get the current time source
std::string multi_crimson_tng::get_time_source(const size_t mboard){
    return _tree->access<std::string>(mb_root(0) / "time_source" / "value").get();
}

// get all possible time sources
std::vector<std::string> multi_crimson_tng::get_time_sources(const size_t mboard){
    return _tree->access<std::vector<std::string> >(mb_root(0) / "time_source" / "options").get();
}

// set the current clock source (Crimson only uses internal reference for now, because it is a really good clock already)
void multi_crimson_tng::set_clock_source(const std::string &source, const size_t mboard){
    //_tree->access<std::string>(mb_root(0) / "clock_source" / "value").set(source);
    return;
}

// get the current clock source
std::string multi_crimson_tng::get_clock_source(const size_t mboard){
    return _tree->access<std::string>(mb_root(0) / "clock_source" / "value").get();
}

// get all possible clock sources
std::vector<std::string> multi_crimson_tng::get_clock_sources(const size_t mboard){
    return _tree->access<std::vector<std::string> >(mb_root(0) / "clock_source" / "options").get();
}

// Set the clock source output, Crimson doesn't have an SMA for clock out
void multi_crimson_tng::set_clock_source_out(const bool enb, const size_t mboard){
    // Not supported
    throw uhd::runtime_error("multi_crimson_tng::set_clock_source_out - not supported on this device");
    return;
}

// Set the time source output, Crimson doesn't have an SMA for time out
void multi_crimson_tng::set_time_source_out(const bool enb, const size_t mboard){
    // Not supported
    throw uhd::runtime_error("multi_crimson_tng::set_time_source_out - not supported on this device");
    return;
}

// Crimson only has support for 1 Digital (mboard) board.
size_t multi_crimson_tng::get_num_mboards(void){
    return 1;
}

sensor_value_t multi_crimson_tng::get_mboard_sensor(const std::string &name, size_t mboard){
    return _tree->access<sensor_value_t>(mb_root(0) / "sensors" / name).get();
}

std::vector<std::string> multi_crimson_tng::get_mboard_sensor_names(size_t mboard){
    return _tree->list(mb_root(0) / "sensors");
}

void multi_crimson_tng::set_user_register(const boost::uint8_t addr, const boost::uint32_t data, size_t mboard){
    // Not implemented
    throw uhd::not_implemented_error("timed command feature not implemented on this hardware");
}

/*******************************************************************
 * RX methods
 ******************************************************************/
rx_streamer::sptr multi_crimson_tng::get_rx_stream(const stream_args_t &args) {
    _check_tng_link_rate(args);
    return this->get_device()->get_rx_stream(args);
}

// Crimson does not support changing subdev properties because you can't add daughter boards
void multi_crimson_tng::set_rx_subdev_spec(const subdev_spec_t &spec, size_t mboard){
    throw uhd::runtime_error("multi_crimson_tng::set_tx_subdev_spec - not supported on this device");
}

// Get the current RX chain subdev properties, Crimson only has one mboard
subdev_spec_t multi_crimson_tng::get_rx_subdev_spec(size_t mboard){
    return subdev_spec_t("Slot_1:RX_Chain_1 Slot_2:RX_Chain_2 Slot_3:RX_Chain_3 Slot_4:RX_Chain_4");
}

// Get number of RX channels, Crimson has 4
size_t multi_crimson_tng::get_rx_num_channels(void){
    return 4;
}

// Get the name of the Crimson subdevice on specified channel
std::string multi_crimson_tng::get_rx_subdev_name(size_t chan){
    return "Channel " + boost::lexical_cast<std::string>(chan);
}

// Set the current RX sampling rate on specified channel
void multi_crimson_tng::set_rx_rate(double rate, size_t chan){
    _tree->access<double>(rx_dsp_root(chan) / "rate" / "value").set(rate);

    double actual_rate = _tree->access<double>(rx_dsp_root(chan) / "rate" / "value").get();

    boost::format base_message (
            "RX Sample Rate Request:\n"
    	    "  Requested sample rate: %f MSps\n"
            "  Actual sample rate: %f MSps\n");
    base_message % (rate/1e6) % (actual_rate/1e6);
    std::string results_string = base_message.str();

    UHD_MSG(status) << results_string;
}

// Get the current RX sampling rate on specified channel
double multi_crimson_tng::get_rx_rate(size_t chan){
    return _tree->access<double>(rx_dsp_root(chan) / "rate" / "value").get();
}

// get the range of possible RX rates on specified channel
meta_range_t multi_crimson_tng::get_rx_rates(size_t chan){
    return _tree->access<meta_range_t>(rx_dsp_root(chan) / "rate" / "range").get();
}

// set the RX frequency on specified channel
tune_result_t multi_crimson_tng::set_rx_freq(const tune_request_t &tune_request, size_t chan) {
    tune_request_t req = tune_request;
    tune_result_t result;
    bool offset = false;

    // pointer to the frequency we use
    double* freq;
    if ( req.rf_freq_policy == tune_request_t::POLICY_MANUAL )
       freq = &(req.rf_freq);
    else
       freq = &(req.target_freq);

    // switch bands if less/greater than 100 MHz
    if (*freq > 100000000.0) _tree->access<int>(rx_rf_fe_root(chan) / "freq" / "band").set(1);
    else                     _tree->access<int>(rx_rf_fe_root(chan) / "freq" / "band").set(0);
    _tree->access<double>(rx_rf_fe_root(chan) / "atten" / "value").set(127);	// set state tree to maximum attenuation

    // offset it by 15 MHz if sampling rate is low.
    double cur_rx_rate = get_rx_rate(chan);
    if (*freq > 15000000.0 && !(cur_rx_rate > (CRIMSON_MASTER_CLOCK_RATE / 9)) ) {
       *freq -= 15000000.0;
       offset = true;
       _tree->access<double>(rx_dsp_root(chan) / "nco").set(-15000000);
    } else {
       _tree->access<double>(rx_dsp_root(chan) / "nco").set(0);
    }

    // check the tuning ranges first, and clip if necessary
    meta_range_t rf_range  = _tree->access<meta_range_t>(rx_rf_fe_root(chan) / "freq" / "range").get();
    meta_range_t dsp_range = _tree->access<meta_range_t>(rx_dsp_root(chan)   / "freq" / "range").get();
    if (req.rf_freq < rf_range.start())	req.rf_freq 	= rf_range.start();
    if (req.rf_freq > rf_range.stop())		req.rf_freq 	= rf_range.stop();
    if (req.dsp_freq < dsp_range.start())	req.dsp_freq	= dsp_range.start();
    if (req.dsp_freq > dsp_range.stop())	req.dsp_freq	= dsp_range.stop();
    if (req.target_freq < rf_range.start())	req.target_freq = rf_range.start();
    if (req.target_freq > rf_range.stop())	req.target_freq = rf_range.stop();

    // use the DSP NCO with base band
    if (_tree->access<int>(rx_rf_fe_root(chan) / "freq" / "band").get() == 0) {
        double cur_dsp_nco = _tree->access<double>(rx_dsp_root(chan) / "nco").get();
        double set_dsp_nco = cur_dsp_nco - *freq;
        _tree->access<double>(rx_dsp_root(chan) / "nco").set(set_dsp_nco);

        result.actual_rf_freq = -set_dsp_nco;

    // use the LO with high band
    } else {
        _tree->access<double>(rx_rf_fe_root(chan) / "freq" / "value").set(*freq);

        // read back the frequency and adjust for the errors with DSP NCO if possible
        double cur_lo_freq = _tree->access<double>(rx_rf_fe_root(chan) / "freq" / "value").get();
        double cur_dsp_nco = _tree->access<double>(rx_dsp_root(chan) / "nco").get();
        double set_dsp_nco = cur_dsp_nco - (*freq - cur_lo_freq);

        if (set_dsp_nco >  161000000) set_dsp_nco = 161000000;
        if (set_dsp_nco < -161000000) set_dsp_nco = -161000000;
        _tree->access<double>(rx_dsp_root(chan) / "nco").set(set_dsp_nco);

        result.actual_rf_freq = cur_lo_freq - set_dsp_nco;
    }

    // account back for the offset
    if (offset)
       *freq += 15000000.0;

    req.dsp_freq = result.actual_rf_freq;
    result.actual_dsp_freq = req.dsp_freq;   // no DSP freq tuning it possible

    result.target_rf_freq  = result.actual_rf_freq;
    result.clipped_rf_freq = result.actual_rf_freq;
    result.target_dsp_freq = result.actual_dsp_freq;

    // print out tuning messages
    boost::format base_message ("RX Tune Request: %f MHz\n");
    base_message % (req.target_freq / 1e6);
    std::string results_string = base_message.str();

    // results of tuning RF LO
    if (result.target_rf_freq != req.target_freq) {
            boost::format rf_lo_message(
                "  The RF LO does not support the requested frequency:\n"
                "    Requested LO Frequency: %f MHz\n"
                "    RF LO Result: %f MHz\n");
            rf_lo_message % (req.target_freq / 1e6) % (result.actual_rf_freq / 1e6);
            results_string += rf_lo_message.str();
    } else {
            boost::format rf_lo_message(
                "  The RF LO supports the requested frequency:\n"
                "    Requested LO Frequency: %f MHz\n"
                "    RF LO Result: %f MHz\n");
            rf_lo_message % (req.target_freq / 1e6) % (result.actual_rf_freq / 1e6);
            results_string += rf_lo_message.str();
    }

    // results of tuning DSP
    if (result.target_dsp_freq != req.target_freq) {
            boost::format dsp_lo_message(
                "  The DSP does not support the requested frequency:\n"
                "    Requested DSP Frequency: %f MHz\n"
                "    DSP Result: %f MHz\n");
            dsp_lo_message % (req.target_freq / 1e6) % (result.actual_dsp_freq / 1e6);
            results_string += dsp_lo_message.str();
    } else {
            boost::format dsp_lo_message(
                "  The DSP supports the requested frequency:\n"
                "    Requested DSP Frequency: %f MHz\n"
                "    DSP Result: %f MHz\n");
            dsp_lo_message % (req.target_freq / 1e6) % (result.actual_dsp_freq / 1e6);
            results_string += dsp_lo_message.str();
    }

    UHD_MSG(status) << results_string;

    // tune_request.args are ignored for Crimson
    return result;
}

// get the RX frequency on specified channel
double multi_crimson_tng::get_rx_freq(size_t chan){
    double cur_dsp_nco = _tree->access<double>(rx_dsp_root(chan) / "nco").get();
    double cur_lo_freq = 0;
    if (_tree->access<int>(rx_rf_fe_root(chan) / "freq" / "band").get() == 1) {
        cur_lo_freq = _tree->access<double>(rx_rf_fe_root(chan) / "freq" / "value").get();
    }
    return cur_lo_freq - cur_dsp_nco;
}

// get the RX frequency range on specified channel
freq_range_t multi_crimson_tng::get_rx_freq_range(size_t chan){
    return _tree->access<meta_range_t>(rx_dsp_root(chan) / "freq" / "range").get();
}

// get front end RX frequency on specified channel
freq_range_t multi_crimson_tng::get_fe_rx_freq_range(size_t chan){
    return _tree->access<meta_range_t>(rx_rf_fe_root(chan) / "freq" / "range").get();
}

// set RX frontend gain on specified channel, name specifies which IC to configure the gain for
void multi_crimson_tng::set_rx_gain(double gain, const std::string &name, size_t chan){
	double MIN_GAIN = 0;
	double MAX_GAIN = 63.25;

	// Sanitize Gain value received
	if 		(gain > MAX_GAIN) 	gain = MAX_GAIN;
	else if (gain < MIN_GAIN) 	gain = MIN_GAIN;



	if (_tree->access<int>(rx_rf_fe_root(chan) / "freq" / "band").get() == 0) {	// Check Channel's Band
		MAX_GAIN = 31.5;	// Max Low Band Gain

		// Sanitize Gain value received for low band
		if (gain > MAX_GAIN) gain = MAX_GAIN;

		// Convert Gain float to value MCU understands
		double gain_token = round(gain / 0.5);	// Max 63
		gain_token = gain_token * 2;			// Max 126

		// Ensure RX Attenuator is in Max Attenuation
		_tree->access<double>(rx_rf_fe_root(chan) / "atten" / "value").set(127);

		// Set Low Band RX Gain
		_tree->access<double>(rx_rf_fe_root(chan) / "gain" / "value").set(gain_token);

	} else {
		// Convert Gain float to value MCU understands
		double gain_token = round(gain / 0.25); // Max 253

		// 0   -> 126	attenuation only
		// 127			0dB
		// 128 -> 253	gain with some attenuation to maintain 0.25dB resolution

		if (gain_token < 127) {	// attenuation only
			_tree->access<double>(rx_rf_fe_root(chan) / "atten" / "value").set(127 - gain_token);
			_tree->access<double>(rx_rf_fe_root(chan) / "gain" / "value").set(0);		// set minimum gain
		} else {
			gain_token = gain_token - 127;	// isolate gain part

			// adjust attenuator to maintain 0.25dB resolution
			if (fmod(gain_token, 2) == 1) {		// odd (0.25 or 0.75)
				_tree->access<double>(rx_rf_fe_root(chan) / "atten" / "value").set(1);
				gain_token++;
			} else {
				_tree->access<double>(rx_rf_fe_root(chan) / "atten" / "value").set(0);
			}

			_tree->access<double>(rx_rf_fe_root(chan) / "gain" / "value").set(gain_token);
		}
	}
}

// get RX frontend gain on specified channel
double multi_crimson_tng::get_rx_gain(const std::string &name, size_t chan){

    double gain_val  = _tree->access<double>(rx_rf_fe_root(chan) / "gain"  / "value").get();
    double atten_val = _tree->access<double>(rx_rf_fe_root(chan) / "atten" / "value").get();

    gain_val  = 126 - gain_val;		// invert gain  scale
    atten_val = 127 - atten_val;	// invert atten scale

    return gain_val + atten_val;
}

// get RX frontend gain range on specified channel
gain_range_t multi_crimson_tng::get_rx_gain_range(const std::string &name, size_t chan){
    return _tree->access<meta_range_t>(rx_rf_fe_root(chan) / "gain" / "range").get();
}

// get RX frontend gain names/options. There is only one configurable gain on the RX rf chain.
std::vector<std::string> multi_crimson_tng::get_rx_gain_names(size_t chan){
    throw uhd::runtime_error("multi_crimson_tng::get_rx_gain_names - not supported on this device");
}

// Crimson does not cater to antenna specifications, this is up to the user to accomodate for
void multi_crimson_tng::set_rx_antenna(const std::string &ant, size_t chan){
    throw uhd::runtime_error("multi_crimson_tng::set_rx_antenna - not supported on this device");
}

// Crimson does not cater to antenna specifications, this is up to the user to accomodate for
std::string multi_crimson_tng::get_rx_antenna(size_t chan){
    throw uhd::runtime_error("multi_crimson_tng::get_rx_antenna - not supported on this device");
}

// Crimson does not cater to antenna specifications, this is up to the user to accomodate for
std::vector<std::string> multi_crimson_tng::get_rx_antennas(size_t chan){
    throw uhd::runtime_error("multi_crimson_tng::get_rx_antennas - not supported on this device");
}

// Set the RX bandwidth on specified channel
void multi_crimson_tng::set_rx_bandwidth(double bandwidth, size_t chan){
    _tree->access<double>(rx_dsp_root(chan) / "bw" / "value").set(bandwidth);
}
// Get the RX bandwidth on specified channel
double multi_crimson_tng::get_rx_bandwidth(size_t chan){
    return _tree->access<double>(rx_dsp_root(chan) / "bw" / "value").get();
}
// Get the RX bandwidth range on specified channel
meta_range_t multi_crimson_tng::get_rx_bandwidth_range(size_t chan){
    return _tree->access<meta_range_t>(rx_dsp_root(chan) / "bw" / "range").get();
}

// There is no dboard interface available for Crimson. Everything is communicated through the mboard (Digital board)
dboard_iface::sptr multi_crimson_tng::get_rx_dboard_iface(size_t chan){
    throw uhd::runtime_error("multi_crimson_tng::get_rx_dboard_iface - not supported on this device");
}

sensor_value_t multi_crimson_tng::get_rx_sensor(const std::string &name, size_t chan){
    return _tree->access<sensor_value_t>(rx_rf_fe_root(0) / "sensors" / name).get();
}

std::vector<std::string> multi_crimson_tng::get_rx_sensor_names(size_t chan){
    return _tree->list(rx_rf_fe_root(0) / "sensors");
}

// Enable dc offset on specified channel
void multi_crimson_tng::set_rx_dc_offset(const bool enb, size_t chan){
    _tree->access<bool>(mb_root(0) / "rx_frontends" / chan_to_string(chan) / "dc_offset" / "enable").set(enb);
}
// Set dc offset on specified channel
void multi_crimson_tng::set_rx_dc_offset(const std::complex<double> &offset, size_t chan){
    _tree->access< std::complex<double> >(mb_root(0) / "rx_frontends" / chan_to_string(chan) / "dc_offset" / "value").set(offset);
}
// set iq balance on specified channel
void multi_crimson_tng::set_rx_iq_balance(const std::complex<double> &offset, size_t chan){
    _tree->access< std::complex<double> >(mb_root(0) / "rx_frontends" / chan_to_string(chan) / "iq_balance" / "value").set(offset);
}

/*******************************************************************
 * TX methods
 ******************************************************************/
tx_streamer::sptr multi_crimson_tng::get_tx_stream(const stream_args_t &args) {
    _check_tng_link_rate(args);
    return this->get_device()->get_tx_stream(args);
}

// Crimson does not support changing subdev properties because you can't add daughter boards
void multi_crimson_tng::set_tx_subdev_spec(const subdev_spec_t &spec, size_t mboard){
    throw uhd::runtime_error("multi_crimson_tng::set_tx_subdev_spec - not supported on this device");
}

// Get the current TX chain subdev properties, Crimson only has one mboard
subdev_spec_t multi_crimson_tng::get_tx_subdev_spec(size_t mboard){
    return subdev_spec_t("Slot_1:TX_Chain_1 Slot_2:TX_Chain_2 Slot_3:TX_Chain_3 Slot_4:TX_Chain_4");
}

// Get number of TX channels, Crimson has 4
size_t multi_crimson_tng::get_tx_num_channels(void){
    return 4;
}

// Get the name of the Crimson subdevice on specified channel
std::string multi_crimson_tng::get_tx_subdev_name(size_t chan){
    return "Channel " + boost::lexical_cast<std::string>(chan);
}

// Set the current TX sampling rate on specified channel
void multi_crimson_tng::set_tx_rate(double rate, size_t chan){
    _tree->access<double>(tx_dsp_root(chan) / "rate" / "value").set(rate);

    double actual_rate = _tree->access<double>(tx_dsp_root(chan) / "rate" / "value").get();

    boost::format base_message (
            "TX Sample Rate Request:\n"
    	    "  Requested sample rate: %f MSps\n"
            "  Actual sample rate: %f MSps\n");
    base_message % (rate/1e6) % (actual_rate/1e6);
    std::string results_string = base_message.str();

    UHD_MSG(status) << results_string;
}

// Get the current TX sampling rate on specified channel
double multi_crimson_tng::get_tx_rate(size_t chan){
    return _tree->access<double>(tx_dsp_root(chan) / "rate" / "value").get();
}

// get the range of possible TX rates on specified channel
meta_range_t multi_crimson_tng::get_tx_rates(size_t chan){
    return _tree->access<meta_range_t>(tx_dsp_root(chan) / "rate" / "range").get();
}

// set the TX frequency on specified channel
tune_result_t multi_crimson_tng::set_tx_freq(const tune_request_t &tune_request, size_t chan){
    tune_request_t req = tune_request;
    tune_result_t result;
    bool offset = false;

    // pointer to the frequency we use
    double* freq;
    if ( req.rf_freq_policy == tune_request_t::POLICY_MANUAL )
       freq = &(req.rf_freq);
    else
       freq = &(req.target_freq);

    // switch bands if less/greater than 100 MHz
    if (*freq > 100000000.0) _tree->access<int>(tx_rf_fe_root(chan) / "freq" / "band").set(1);
    else                     _tree->access<int>(tx_rf_fe_root(chan) / "freq" / "band").set(0);
    _tree->access<double>(tx_rf_fe_root(chan) / "gain" / "value").set(0);	// Set state tree to min gain

    // offset it by 85 MHz if sampling rate is low.
    double cur_tx_rate = get_tx_rate(chan);
    if (*freq > 85000000.0 && !(cur_tx_rate > (CRIMSON_MASTER_CLOCK_RATE / 9))) {
       *freq -= 85000000.0;
       offset = true;
       _tree->access<double>(tx_rf_fe_root(chan) / "nco").set(85000000);
    } else {
       _tree->access<double>(tx_rf_fe_root(chan) / "nco").set(0);
    }

    // check the tuning ranges first, and clip if necessary
    meta_range_t rf_range  = _tree->access<meta_range_t>(tx_rf_fe_root(chan) / "freq" / "range").get();
    meta_range_t dsp_range = _tree->access<meta_range_t>(tx_dsp_root(chan)   / "freq" / "range").get();
    if (req.rf_freq < rf_range.start())	req.rf_freq 	= rf_range.start();
    if (req.rf_freq > rf_range.stop())		req.rf_freq 	= rf_range.stop();
    if (req.dsp_freq < dsp_range.start())	req.dsp_freq	= dsp_range.start();
    if (req.dsp_freq > dsp_range.stop())	req.dsp_freq	= dsp_range.stop();
    if (req.target_freq < rf_range.start())	req.target_freq = rf_range.start();
    if (req.target_freq > rf_range.stop())	req.target_freq = rf_range.stop();

    // use the DSP NCO with base band
    if (_tree->access<int>(tx_rf_fe_root(chan) / "freq" / "band").get() == 0) {
        double cur_dac_nco = _tree->access<double>(tx_rf_fe_root(chan) / "nco").get();
        double set_dsp_nco = *freq;
        _tree->access<double>(tx_dsp_root(chan) / "nco").set(set_dsp_nco);

        result.actual_rf_freq = set_dsp_nco + cur_dac_nco;

    // use the LO with high band
    } else {
        _tree->access<double>(tx_rf_fe_root(chan) / "freq" / "value").set(*freq);

        // read back the frequency and adjust for the errors with DSP NCO if possible
        double cur_lo_freq = _tree->access<double>(tx_rf_fe_root(chan) / "freq" / "value").get();
        double cur_dac_nco = _tree->access<double>(tx_rf_fe_root(chan) / "nco").get();
        double set_dsp_nco = *freq - cur_lo_freq;

        if (set_dsp_nco >  161000000) set_dsp_nco = 161000000;
        if (set_dsp_nco < -161000000) set_dsp_nco = -161000000;
        _tree->access<double>(tx_dsp_root(chan) / "nco").set(set_dsp_nco);

        result.actual_rf_freq = cur_lo_freq + set_dsp_nco + cur_dac_nco;
    }

    // account back for the offset
    if (offset)
       *freq += 85000000.0;

    req.dsp_freq = result.actual_rf_freq;
    result.actual_dsp_freq = req.dsp_freq;   // no DSP freq tuning it possible

    result.target_rf_freq  = result.actual_rf_freq;
    result.clipped_rf_freq = result.actual_rf_freq;
    result.target_dsp_freq = result.actual_dsp_freq;

    // print out tuning messages
    boost::format base_message ("TX Tune Request: %f MHz\n");
    base_message % (req.target_freq / 1e6);
    std::string results_string = base_message.str();

    // results of tuning RF LO
    if (result.target_rf_freq != req.target_freq) {
            boost::format rf_lo_message(
                "  The RF LO does not support the requested frequency:\n"
                "    Requested LO Frequency: %f MHz\n"
                "    RF LO Result: %f MHz\n");
            rf_lo_message % (req.target_freq / 1e6) % (result.actual_rf_freq / 1e6);
            results_string += rf_lo_message.str();
    } else {
            boost::format rf_lo_message(
                "  The RF LO supports the requested frequency:\n"
                "    Requested LO Frequency: %f MHz\n"
                "    RF LO Result: %f MHz\n");
            rf_lo_message % (req.target_freq / 1e6) % (result.actual_rf_freq / 1e6);
            results_string += rf_lo_message.str();
    }

    // results of tuning DSP
    if (result.target_dsp_freq != req.target_freq) {
            boost::format dsp_lo_message(
                "  The DSP does not support the requested frequency:\n"
                "    Requested DSP Frequency: %f MHz\n"
                "    DSP Result: %f MHz\n");
            dsp_lo_message % (req.target_freq / 1e6) % (result.actual_dsp_freq / 1e6);
            results_string += dsp_lo_message.str();
    } else {
            boost::format dsp_lo_message(
                "  The DSP supports the requested frequency:\n"
                "    Requested DSP Frequency: %f MHz\n"
                "    DSP Result: %f MHz\n");
            dsp_lo_message % (req.target_freq / 1e6) % (result.actual_dsp_freq / 1e6);
            results_string += dsp_lo_message.str();
    }

    UHD_MSG(status) << results_string;

    // tune_request.args are ignored for Crimson
    return result;
}

// get the TX frequency on specified channel
double multi_crimson_tng::get_tx_freq(size_t chan){
    double cur_dac_nco = _tree->access<double>(tx_rf_fe_root(chan) / "nco").get();
    double cur_dsp_nco = _tree->access<double>(tx_dsp_root(chan) / "nco").get();
    double cur_lo_freq = 0;
    if (_tree->access<int>(tx_rf_fe_root(chan) / "freq" / "band").get() == 1) {
    	cur_lo_freq = _tree->access<double>(tx_rf_fe_root(chan) / "freq" / "value").get();
    }
    return cur_lo_freq + cur_dac_nco + cur_dsp_nco;
}

// get the TX frequency on specified channel
freq_range_t multi_crimson_tng::get_tx_freq_range(size_t chan){
    return _tree->access<meta_range_t>(tx_dsp_root(chan) / "freq" / "range").get();
}

// get the TX frequency range on specified channel
freq_range_t multi_crimson_tng::get_fe_tx_freq_range(size_t chan){
    return _tree->access<meta_range_t>(tx_rf_fe_root(chan) / "freq" / "range").get();
}

// set TX frontend gain on specified channel, name specifies which IC to configure the gain for
void multi_crimson_tng::set_tx_gain(double gain, const std::string &name, size_t chan){
	double MAX_GAIN = 31.75;
	double MIN_GAIN = 0;

	if 		(gain > MAX_GAIN) gain = MAX_GAIN;
	else if (gain < MIN_GAIN) gain = MIN_GAIN;

	gain = round(gain / 0.25);

    _tree->access<double>(tx_rf_fe_root(chan) / "gain" / "value").set(gain);
}

// get TX frontend gain on specified channel
double multi_crimson_tng::get_tx_gain(const std::string &name, size_t chan){
    double gain_val = _tree->access<double>(tx_rf_fe_root(chan) / "gain" / "value").get();
    return gain_val * 0.25;
}

// get TX frontend gain range on specified channel
gain_range_t multi_crimson_tng::get_tx_gain_range(const std::string &name, size_t chan){
    return _tree->access<meta_range_t>(tx_rf_fe_root(chan) / "gain" / "range").get();
}

// get TX frontend gain names/options. There is only one configurable gain on the TX rf chain.
std::vector<std::string> multi_crimson_tng::get_tx_gain_names(size_t chan){
    throw uhd::runtime_error("multi_crimson_tng::get_tx_gain_names - not supported on this device");
}

// Crimson does not cater to antenna specifications, this is up to the user to accomodate for
void multi_crimson_tng::set_tx_antenna(const std::string &ant, size_t chan){
    throw uhd::runtime_error("multi_crimson_tng::set_tx_antenna - not supported on this device");
}

// Crimson does not cater to antenna specifications, this is up to the user to accomodate for
std::string multi_crimson_tng::get_tx_antenna(size_t chan){
    throw uhd::runtime_error("multi_crimson_tng::get_tx_antenna - not supported on this device");
}

// Crimson does not cater to antenna specifications, this is up to the user to accomodate for
std::vector<std::string> multi_crimson_tng::get_tx_antennas(size_t chan){
    throw uhd::runtime_error("multi_crimson_tng::get_tx_antennas - not supported on this device");
}

// Set the TX bandwidth on specified channel
void multi_crimson_tng::set_tx_bandwidth(double bandwidth, size_t chan){
    _tree->access<double>(tx_dsp_root(chan) / "bw" / "value").set(bandwidth);
}

// Get the TX bandwidth on specified channel
double multi_crimson_tng::get_tx_bandwidth(size_t chan){
    return _tree->access<double>(tx_dsp_root(chan) / "bw" / "value").get();
}

// Get the TX bandwidth range on specified channel
meta_range_t multi_crimson_tng::get_tx_bandwidth_range(size_t chan){
    return _tree->access<meta_range_t>(tx_dsp_root(chan) / "bw" / "range").get();
}

// There is no dboard interface available for Crimson. Everything is communicated through the mboard (Digital board)
dboard_iface::sptr multi_crimson_tng::get_tx_dboard_iface(size_t chan){
    throw uhd::runtime_error("multi_crimson_tng::get_tx_dboard_iface - not supported on this device");
}

sensor_value_t multi_crimson_tng::get_tx_sensor(const std::string &name, size_t chan){
    return _tree->access<sensor_value_t>(tx_rf_fe_root(chan) / "sensors" / name).get();
}

std::vector<std::string> multi_crimson_tng::get_tx_sensor_names(size_t chan){
    return _tree->list(tx_rf_fe_root(chan) / "sensors");
}

// Set dc offset on specified channel
void multi_crimson_tng::set_tx_dc_offset(const std::complex<double> &offset, size_t chan){
    _tree->access< std::complex<double> >(tx_rf_fe_root(chan) / "dc_offset" / "value").set(offset);
}

// set iq balance on specified channel
void multi_crimson_tng::set_tx_iq_balance(const std::complex<double> &offset, size_t chan){
    _tree->access< std::complex<double> >(tx_rf_fe_root(chan) / "iq_balance" / "value").set(offset);
}

/*******************************************************************
 * GPIO methods
 ******************************************************************/
std::vector<std::string> multi_crimson_tng::get_gpio_banks(const size_t mboard){
    // Not supported
    throw uhd::runtime_error("multi_crimson_tng::get_gpio_banks - not supported on this device");
}

void multi_crimson_tng::set_gpio_attr(const std::string &bank, const std::string &attr,
    const boost::uint32_t value, const boost::uint32_t mask, const size_t mboard){
    // Not supported
    throw uhd::runtime_error("multi_crimson_tng::set_gpio_attr - not supported on this device");
}

boost::uint32_t multi_crimson_tng::get_gpio_attr(const std::string &bank, const std::string &attr, const size_t mboard){
    // Not supported
    throw uhd::runtime_error("multi_crimson_tng::get_gpio_attr - not supported on this device");
}

/*******************************************************************
 * Helper methods
 ******************************************************************/
// Getting FS paths
fs_path multi_crimson_tng::mb_root(const size_t mboard)
{
    try
    {
        const std::string name = _tree->list("/mboards").at(mboard);
        return "/mboards/" + name;
    }
    catch(const std::exception &e)
    {
        throw uhd::index_error(str(boost::format("multi_usrp::mb_root(%u) - %s") % mboard % e.what()));
    }
}

fs_path multi_crimson_tng::rx_rf_fe_root(const size_t chan) {
    size_t channel;
    if (chan > CRIMSON_RX_CHANNELS) 	channel = 0;
    else				channel = chan;

    return mb_root(0) / "dboards" / chan_to_alph(channel) / "rx_frontends" / chan_to_string(channel);
}

fs_path multi_crimson_tng::rx_dsp_root(const size_t chan) {
    size_t channel;
    if (chan > CRIMSON_RX_CHANNELS) 	channel = 0;
    else				channel = chan;
    return mb_root(0) / "rx_dsps" / chan_to_string(channel);
}

fs_path multi_crimson_tng::rx_link_root(const size_t chan) {
    size_t channel;
    if (chan > CRIMSON_RX_CHANNELS) 	channel = 0;
    else				channel = chan;
    return mb_root(0) / "rx_link" / chan_to_string(channel);
}

fs_path multi_crimson_tng::tx_rf_fe_root(const size_t chan) {
    size_t channel;
    if (chan > CRIMSON_TX_CHANNELS) 	channel = 0;
    else				channel = chan;

    return mb_root(0) / "dboards" / chan_to_alph(channel) / "tx_frontends" / chan_to_string(channel);
}

fs_path multi_crimson_tng::tx_dsp_root(const size_t chan) {
    size_t channel;
    if (chan > CRIMSON_TX_CHANNELS) 	channel = 0;
    else				channel = chan;
    return mb_root(0) / "tx_dsps" / chan_to_string(channel);
}

fs_path multi_crimson_tng::tx_link_root(const size_t chan) {
    size_t channel;
    if (chan > CRIMSON_TX_CHANNELS) 	channel = 0;
    else				channel = chan;
    return mb_root(0) / "tx_link" / chan_to_string(channel);
}

// Channel string handling
std::string multi_crimson_tng::chan_to_string(size_t chan) {
    return "Channel_" + boost::lexical_cast<std::string>((char)(chan + 65));
}

std::string multi_crimson_tng::chan_to_alph(size_t chan) {
    return boost::lexical_cast<std::string>((char)(chan + 65));
}
