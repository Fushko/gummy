#include <iostream>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <plog/Init.h>
#include <plog/Appenders/RollingFileAppender.h>

#include "../commons/defs.h"
#include "../commons/utils.h"
#include "cfg.h"

#include "xorg.h"
#include "screenctl.h"
#include "sysfs.h"

using std::cout;

struct Options {
	Options(std::string in)
	{
		json msg = json::parse(in);
		scr_no          = msg["scr_no"];
		brt_perc        = msg["brt_perc"];
		temp_k          = msg["temp_k"];
		brt_auto        = msg["brt_mode"];
		brt_auto_min    = msg["brt_auto_min"];
		brt_auto_max    = msg["brt_auto_max"];
		brt_auto_offset = msg["brt_auto_offset"];
		temp_auto       = msg["temp_mode"];
		sunrise_time    = msg["sunrise_time"];
		sunset_time     = msg["sunset_time"];
		temp_adaptation_time = msg["temp_adaptation_time"];
	}
	int scr_no          = -1;
	int brt_perc        = -1;
	int temp_k          = -1;
	int brt_auto        = -1;
	int brt_auto_min    = -1;
	int brt_auto_max    = -1;
	int brt_auto_offset = -1;
	int temp_auto       = -1;
	int temp_adaptation_time = -1;
	std::string sunrise_time;
	std::string sunset_time;
};

void readMessages(Xorg &xorg, ScreenCtl &screenctl)
{
	mkfifo(fifo_name, S_IFIFO|0640);

	while (1) {
		int fd = open(fifo_name, O_RDONLY);
		char rdbuf[255];

		int len = read(fd, rdbuf, sizeof(rdbuf));
		rdbuf[len] = '\0';

		if (strcmp(rdbuf, "stop") == 0) {
			close(fd);
			break;
		}

		Options opts(rdbuf);

		if (!opts.sunrise_time.empty()) {
			cfg["temp_auto_sunrise"] = opts.sunrise_time;
			screenctl.notifyTemp();
		}

		if (!opts.sunset_time.empty()) {
			cfg["temp_auto_sunset"] = opts.sunset_time;
			screenctl.notifyTemp();
		}

		if (opts.temp_adaptation_time != -1) {
			cfg["temp_auto_speed"] = opts.temp_adaptation_time;
			screenctl.notifyTemp();
		}

		if (opts.scr_no == -1) {

			if (opts.temp_k != -1) {
				cfg["temp_auto"] = false;
				screenctl.notifyTemp();
			} else if (opts.temp_auto != -1) {
				cfg["temp_auto"] = bool(opts.temp_auto);
				screenctl.notifyTemp();
			}

			for (int i = 0; i < xorg.screenCount(); ++i) {

				if (opts.temp_k != -1) {
					cfg["screens"][i]["temp_step"] = int(remap(opts.temp_k, temp_k_min, temp_k_max, 0, temp_steps_max));
				}

				if (opts.brt_auto != -1) {
					cfg["screens"][i]["brt_auto"] = bool(opts.brt_auto);
					screenctl.notifyMonitor(i);
				}

				if (opts.brt_perc != -1 && opts.brt_auto != 1) {
					cfg["screens"][i]["brt_step"] = int(remap(opts.brt_perc, 0, 100, 0, brt_steps_max));
				}

				if (opts.brt_auto_min != -1) {
					cfg["screens"][i]["brt_auto_min"] = int(remap(opts.brt_auto_min, 0, 100, 0, brt_steps_max));
				}

				if (opts.brt_auto_max != -1) {
					cfg["screens"][i]["brt_auto_max"] = int(remap(opts.brt_auto_max, 0, 100, 0, brt_steps_max));
				}

				if (opts.brt_auto_offset != -1) {
					cfg["screens"][i]["brt_auto_offset"] = int(remap(opts.brt_auto_offset, 0, 100, 0, brt_steps_max));
				}
			}

			xorg.setGamma();
		} else {

			if (opts.scr_no > xorg.screenCount() - 1) {
				cout << "Screen " << opts.scr_no << " not available.\n";
				continue;
			}

			if (opts.brt_auto != -1) {
				cfg["screens"][opts.scr_no]["brt_auto"] = bool(opts.brt_auto);
				screenctl.notifyMonitor(opts.scr_no);
			}

			if (opts.temp_k != -1) {
				cfg["screens"][opts.scr_no]["temp_step"] = int(remap(opts.temp_k, temp_k_min, temp_k_max, 0, temp_steps_max));
				cfg["screens"][opts.scr_no]["temp_auto"] = false;
			} else if (opts.temp_auto != -1) {
				cfg["screens"][opts.scr_no]["temp_auto"] = bool(opts.temp_auto);

				if (opts.temp_auto == 1) {
					cfg["screens"][opts.scr_no]["temp_step"] = screenctl.getAutoTempStep();
				}
			}

			if (opts.brt_auto_min != -1) {
				cfg["screens"][opts.scr_no]["brt_auto_min"] = int(remap(opts.brt_auto_min, 0, 100, 0, brt_steps_max));
			}

			if (opts.brt_auto_max != -1) {
				cfg["screens"][opts.scr_no]["brt_auto_max"] = int(remap(opts.brt_auto_max, 0, 100, 0, brt_steps_max));
			}

			if (opts.brt_auto_offset != -1) {
				cfg["screens"][opts.scr_no]["brt_auto_offset"] = int(remap(opts.brt_auto_offset, 0, 100, 0, brt_steps_max));
			}

			if (opts.brt_perc != -1 && opts.brt_auto != 1) {
				cfg["screens"][opts.scr_no]["brt_step"] = int(remap(opts.brt_perc, 0, 100, 0, brt_steps_max));
			}

			if (opts.temp_k != -1) {
				cfg["screens"][opts.scr_no]["temp_step"] = int(remap(opts.temp_k, temp_k_min, temp_k_max, 0, temp_steps_max));
			}

			xorg.setGamma(
			    opts.scr_no,
			    cfg["screens"][opts.scr_no]["brt_step"],
			    cfg["screens"][opts.scr_no]["temp_step"]
			);
		}

		config::write();
	}
}

int main(int argc, char **argv)
{
	if (argc > 1 && strcmp(argv[1], "-v") == 0) {
		cout << "0.1\n";
		return 0;
	}

	if (alreadyRunning()) {
		return 0;
	}

	static plog::RollingFileAppender<plog::TxtFormatter> f("gummy.log", 1024 * 1024 * 5, 1);
	plog::init(plog::Severity(plog::debug), &f);

	config::read();

	plog::get()->setMaxSeverity(plog::Severity(cfg["log_level"].get<int>()));

	Xorg xorg;

	int new_screens = xorg.screenCount() - cfg["screens"].size();
	if (new_screens > 0) {
		config::addScreenEntries(cfg, new_screens);
		config::write();
	}

	ScreenCtl screenctl(&xorg);

	readMessages(xorg, screenctl);

	cout << "gummy stopped\n";
	return 0;
}
