#!/usr/bin/env python3


import argparse
import yaml
import os
import shutil


def generate_linker_common(filename="vpp_linker.cfg", name='', replica=''):
    if name!='':
        name = 'nk='+name+':'
    if replica=='1':
        replica = ''

    out_str='''
kernel_frequency=0:200|1:200

[advanced]
misc=solution_name=link_project

[connectivity]
@NAME@REPLICA
'''.replace('@NAME',name).replace('@REPLICA',replica)

    with open(filename,"w") as f:
        f.write(out_str)


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
clock=200000000:@NAME@

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



def check_strings(string1,string2):
    list1 = string1.split('_')
    list2 = string2.split('_')
    for i in range(0, min(len(list1),len(list2))-1):
        check_true = (list1[i] == list2[i])
        if check_true == False:
            break

    return check_true


##############################################
##############################################
##############################################

config_file = 'config.yaml'
app_dir = 'dev_apps/'
autogen_dir = 'hw_hls/autogen'

parser = argparse.ArgumentParser()
parser.add_argument('-c', '--config', nargs=1, help=f'yaml config file (default {config_file})')
parser.add_argument('-d', '--target-dir', nargs=1, help=f'referring directory for different apps')
parser.add_argument('--switch-name', help='print mangled name of the Textarossa switch IP, based on configuration', action='store_true')
parser.add_argument('--list-autogen', help='dump list of kernels which are going to be generated', action='store_true')
args = parser.parse_args()

if args.target_dir:
    app_dir=app_dir+args.target_dir[0]
    autogen_dir = app_dir+'/'+autogen_dir

if args.config:
    if args.target_dir:
        config_file = app_dir+'/'+args.config[0]
    else:
        config_file = args.config[0]


with open(config_file) as file:
    params = yaml.safe_load(file)
    kernels = params['kernels']
    config = params['config']

if args.list_autogen:
        one=0
        two=0
        three=0
        for k in kernels:
                        if one==0 or (one==1 and two==0) or (one==1 and two==1 and three==0):
                                print('dispatcher_' + str(k['switch_port']))
                                print('aggregator_' + str(k['switch_port']))
                        if k['switch_port']==1:
                                one=1
                        if k['switch_port']==2:
                                two=1            
                        if k['switch_port']==3:
                                three=1
                            
        exit()



req_in_ports = 0
req_out_ports = 0
name_replica = ''
num_replica = 1
if "links" in config:
    req_out_ports = config['links']
for index,k in enumerate(kernels):
    if 'switch_port' in k:
        req_in_ports = max(req_in_ports, k['switch_port']+1)
    if index < len(kernels)-1:
        check = check_strings(k['name'],kernels[index+1]['name'])
        if check:
            name_replica = k['name']
            num_replica+=1

switch_name = 'TextaRossa_switch_{}in_{}ex'.format(req_in_ports, req_out_ports)
if args.switch_name:
    print(switch_name)
    exit()


string_list=name_replica.split('_')
name=''
for i in range(0,len(string_list)-1):
    name+=string_list[i]
    if(i < len(string_list)-2):
        name+='_'

generate_linker_common("vpp_linker.cfg", name, str(num_replica))

in_channels = [0,0,0,0]
out_channels = [0,0,0,0]

for k in kernels:
    in_channels[k['switch_port']]+=k['input_channels']
    out_channels[k['switch_port']]+=k['output_channels']

print('Generating dispatcher/aggregator for kernel {}'.format(k['name']))
print('Generating a dispatcher with {} output channels...'.format(k['input_channels']))
for i in range(0,req_in_ports):
    generate_dispatcher(i, in_channels[i], autogen_dir)
    generate_aggregator(i, out_channels[i], autogen_dir)

port=999
filename="vpp_linker.cfg"
input_counter=0
output_counter=0
for index,k in enumerate(kernels):
    print('Generating linker...')
    
    krnl_name = k['name']
    dispatcher_name = 'dispatcher_' + str(k['switch_port'])
    aggregator_name = 'aggregator_' + str(k['switch_port'])
    output_channels = k['output_channels']
    input_channels = k['input_channels']


    if(index < len(kernels)-1):
        same = (k['switch_port'] == kernels[index+1]['switch_port']) and (((k['output_channels'] == kernels[index+1]['output_channels'])) or ((k['input_channels'] == kernels[index+1]['input_channels']))) 


    out_str = ''
    if k['switch_port'] != port:
        out_str += stream_connect('TextaRossa_switch', dispatcher_name, switch_port=k['switch_port'])
        out_str += stream_connect(aggregator_name, 'TextaRossa_switch', switch_port=k['switch_port'])
        input_counter=0
        output_counter=0
        port = k['switch_port']

    if input_channels == 1:
        if(same == False):
            out_str += stream_connect(dispatcher_name, krnl_name)
        else:
            out_str += stream_connect(dispatcher_name, krnl_name, channel=f'_{input_counter}')
            input_counter+=1

    elif input_channels > 1:
        for n in range(0, input_channels):
            out_str += stream_connect(dispatcher_name, krnl_name, channel=f'_{input_counter}', kchannel=f'_{n}')
            input_counter+=1


    if output_channels == 1:
        if(same == False):
            out_str += stream_connect(krnl_name,aggregator_name)
        else:
            out_str += stream_connect(krnl_name, aggregator_name, channel=f'_{output_counter}')
            output_counter+=1
    elif output_channels > 1:
        for n in range(0, output_channels):
            out_str += stream_connect(krnl_name, aggregator_name, channel=f'_{output_counter}', kchannel=f'_{n}')
            output_counter+=1

    
    with open(filename,"a") as f:
        f.write(out_str)


print('Copying switch IP with {} input and {} output ports'.format(req_in_ports, req_out_ports))
os.makedirs('hw_hdl', exist_ok=True)
shutil.copyfile('ip_repo/{}.xo'.format(switch_name), 'hw_hdl/{}.xo'.format(switch_name))








