#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2022 INFN APE Lab - Sezione di Roma
# SPDX-License-Identifier: EUPL-1.2

import argparse
import yaml
import os
import shutil


def generate_linker_common(filename="vpp_linker.cfg"):
    out_str='''
kernel_frequency=0:100|1:100

[advanced]
misc=solution_name=link_project

[connectivity]
nk=krnl_sr:4

'''

    with open(filename,"w") as f:
        f.write(out_str)


#def generate_linker(kernel, in_channels1, in_channels2, out_channels1, out_channels2, filename="vpp_linker.cfg"):

def stream_connect(source, dest, channel='', kchannel='', switch_port=''):
     if 'dispatcher' in source:
         source_str = f'{source}_1.fifo_data_out{channel}'
     elif 'aggregator' in source:
         source_str = f'{source}_1.fifo_@SUB@_out{channel}'
     elif 'TextaRossa_switch' in source:
         source_str = f'{source}_1.@SSUB@axisrx{switch_port}'
     
     else:
         source_str = f'{source}.message_@SUB@_out{kchannel}'

     if  'aggregator' in dest:
         dest_str = f'{dest}_1.fifo_data_in{channel}'
     elif  'dispatcher' in dest:
         dest_str = f'{dest}_1.fifo_@SUB@_in{channel}'
     elif 'TextaRossa_switch' in dest:
         dest_str = f'{dest}_1.@SSUB@axistx{switch_port}'
     else:
         dest_str = f'{dest}.message_@SUB@_in{kchannel}'

     out = ''
     out += f'stream_connect={source_str}:{dest_str}\n'.replace('@SUB@', 'data').replace('@SSUB@', 'dt')
     if 'TextaRossa_switch' in source or 'aggregator' in source:
       out += f'stream_connect={source_str}:{dest_str}\n'.replace('@SUB@', 'hdr').replace('@SSUB@', 'hd')

     return out



def generate_kernel_cfg(name, dest_dir="hw_hls/autogen"):
    out_str = '''
kernel=@NAME@

[hls]
clock=100000000:@NAME@

[advanced]
misc=solution_name=@NAME@
'''.replace('@NAME@', name)

    with open(f'{dest_dir}/{name}/{name}.cfg',"w") as f:
        f.write(out_str)

def generate_dispatcher(nport, nchan, dest_dir="hw_hls/autogen"):
    name = 'dispatcher_' + str(nport)
    out_str  = '''
#include "ape_hls/dispatcher.hpp"
#define N_OUTPUT_CHANNEL @NCHAN@
void @NAME@(header_stream_t &fifo_hdr_in,message_stream_t &fifo_data_in/*,header_stream_t fifo_hdr_out[N_OUTPUT_CHANNEL]*/,message_stream_t fifo_data_out[N_OUTPUT_CHANNEL]){
#pragma HLS INTERFACE ap_ctrl_none port=return
//#pragma HLS INTERFACE axis port=fifo_hdr_out
#pragma HLS INTERFACE axis port=fifo_data_out
//#pragma HLS DATAFLOW
    ape::dispatcher_template<N_OUTPUT_CHANNEL, 128, (512+1024)>(fifo_hdr_in, fifo_data_in, /*fifo_hdr_out,*/ fifo_data_out);
}
'''.replace('@NCHAN@', str(nchan)).replace('@NAME@', 'dispatcher_'+str(nport))

    os.makedirs(f'{dest_dir}/{name}', exist_ok=True)
    generate_kernel_cfg(name, dest_dir)
    with open(f'{dest_dir}/{name}/{name}.cpp',"w") as f:
        f.write(out_str)


def generate_aggregator(nport, nchan, dest_dir="hw_hls/autogen"):
    name = 'aggregator_' + str(nport)
    out_str = '''
#include "ape_hls/aggregator.hpp"
#define N_INPUT_CHANNEL @NCHAN@
void @NAME@(unsigned nevents /*header_stream_t fifo_hdr_in[N_INPUT_CHANNEL]*/,message_stream_t fifo_data_in[N_INPUT_CHANNEL],header_stream_t &fifo_hdr_out,message_stream_t &fifo_data_out){
#pragma HLS INTERFACE ap_ctrl_none port=return
//#pragma HLS INTERFACE axis port=fifo_hdr_in
#pragma HLS INTERFACE axis port=fifo_data_in
    ape::aggregator_template<N_INPUT_CHANNEL>(nevents,/*fifo_hdr_in,*/ fifo_data_in, fifo_hdr_out, fifo_data_out);
}
'''.replace('@NCHAN@', str(nchan)).replace('@NAME@', 'aggregator_'+str(nport))

    os.makedirs(f'{dest_dir}/{name}', exist_ok=True)
    generate_kernel_cfg(name, dest_dir)
    with open(f'{dest_dir}/{name}/{name}.cpp',"w") as f:
        f.write(out_str)


##############################################
##############################################
##############################################

config_file = 'config.yaml'

parser = argparse.ArgumentParser()
parser.add_argument('-c', '--config', nargs=1, help=f'yaml config file (default {config_file})')
parser.add_argument('--switch-name', help='print mangled name of the Textarossa switch IP, based on configuration', action='store_true')
parser.add_argument('--list-autogen', help='dump list of kernels which are going to be generated', action='store_true')
args = parser.parse_args()

if args.config:
    config_file = args.config[0]

with open(config_file) as file:
    params = yaml.safe_load(file)
    kernels = params['kernels']
    config = params['config']

if args.list_autogen:
	one=0
	two=0
	for k in kernels:
			if one==0 or (one==1 and two==0):
				print('dispatcher_' + str(k['switch_port']))
				print('aggregator_' + str(k['switch_port']))
			if k['switch_port']==1:
				one=1
			if k['switch_port']==2:
				two
	exit()


req_in_ports = 0
req_out_ports = 0
if "links" in config:
    req_out_ports = config['links']
for k in kernels:
    if 'switch_port' in k:
        req_in_ports = max(req_in_ports, k['switch_port']+1)
switch_name = 'TextaRossa_switch_{}in_{}ex'.format(req_in_ports, req_out_ports)
if args.switch_name:
    print(switch_name)
    exit()


generate_linker_common()
in_channels0=0
in_channels1=0
in_channels2=0
in_channels3=0
out_channels0=0
out_channels1=0
out_channels2=0
out_channels3=0
for k in kernels:
	if k['switch_port']==0:
		in_channels0+=k['input_channels'];
		out_channels0+=k['output_channels'];
	if k['switch_port']==1:
		in_channels1+=k['input_channels'];
		out_channels1+=k['output_channels'];
	if k['switch_port']==2:
		in_channels2+=k['input_channels'];
		out_channels2+=k['output_channels']; 
	if k['switch_port']==3:
		in_channels3+=k['input_channels'];
		out_channels3+=k['output_channels'];

print('Generating dispatcher/aggregator for kernel {}'.format(k['name']))
print('Generating a dispatcher with {} output channels...'.format(k['input_channels']))
generate_dispatcher(0, in_channels0)
generate_dispatcher(1, in_channels1)
generate_dispatcher(2, in_channels2)
generate_dispatcher(3, in_channels3)

print('Generating an aggregator with {} input channels...'.format(k['output_channels']))
generate_aggregator(0, out_channels0)
generate_aggregator(1, out_channels1)
generate_aggregator(2, out_channels2)
generate_aggregator(3, out_channels3)

port=999
filename="vpp_linker.cfg"
inn=0
outt=0
for k in kernels:
	print('Generating linker...')
	#generate_linker(k, in_channels1, in_channels2, out_channels1, out_channels2)
	
	krnl_name = k['name']
	dispatcher_name = 'dispatcher_' + str(k['switch_port'])
	aggregator_name = 'aggregator_' + str(k['switch_port'])
	output_channels = k['output_channels']
	input_channels = k['input_channels']

    #if output_channels < 1 or input_channels < 1:
     #   print("ERROR on channels number!")
      #  return

	out_str = ''
	if k['switch_port']!= port:
   		out_str += stream_connect('TextaRossa_switch', dispatcher_name, switch_port=k['switch_port'])
   		out_str += stream_connect(aggregator_name, 'TextaRossa_switch', switch_port=k['switch_port'])
   		port=k['switch_port']
   		if k['switch_port']== 0:
   			inn=0
   			outt=0
   		elif k['switch_port']== 1:
   			inn=0
   			outt=0
   		elif k['switch_port']== 2:
   			inn=0
   			outt=0     
   		elif k['switch_port']== 3:
   			inn=0
   			outt=0

	if input_channels == 1:
			if(port!=0):
			 out_str += stream_connect(dispatcher_name, krnl_name,channel=f'_{inn}')
			else:
			 out_str += stream_connect(dispatcher_name, krnl_name)
			inn+=1;
        #out_str+='stream_connect=dispatcher_1.fifo_data_out:krnl_enea_1.message_data_in\n'
        #out_str+='stream_connect=dispatcher_1.fifo_hdr_out:krnl_enea_1.message_hdr_in\n'
	elif input_channels == 0:
 	 	 	print('ZERO');
	else:
			for n in range(0,input_channels):
							out_str += stream_connect(dispatcher_name, krnl_name, channel=f'_{inn}', kchannel=f'_{n}')
							inn+=1

           
	out_str+='\n'

	if output_channels == 1:
        	if(port!=0):
        		out_str += stream_connect(krnl_name, aggregator_name, channel=f'_{outt}')
        	else:
        		out_str += stream_connect(krnl_name, aggregator_name)
        	outt+=1
       
	elif output_channels == 0:
    	   print('ZERO');
	else:
			for n in range(outt,output_channels+outt):
							out_str += stream_connect(krnl_name, aggregator_name, channel=f'_{outt}', kchannel=f'_{n}')
							outt+=1
			
	
	with open(filename,"a") as f:
			f.write(out_str)


print('Copying switch IP with {} input and {} output ports'.format(req_in_ports, req_out_ports))
os.makedirs('hw_hdl', exist_ok=True)
shutil.copyfile('ip_repo/{}.xo'.format(switch_name), 'hw_hdl/{}.xo'.format(switch_name))


