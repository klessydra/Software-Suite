#!/bin/bash

#	██████╗░██╗░██████╗░░░░░██╗░█████╗░██╗███╗░░██╗████████╗
#	██╔══██╗██║██╔════╝░░░░░██║██╔══██╗██║████╗░██║╚══██╔══╝
#	██║░░██║██║╚█████╗░░░░░░██║██║░░██║██║██╔██╗██║░░░██║░░░
#	██║░░██║██║░╚═══██╗██╗░░██║██║░░██║██║██║╚████║░░░██║░░░
#	██████╔╝██║██████╔╝╚█████╔╝╚█████╔╝██║██║░╚███║░░░██║░░░
#	╚═════╝░╚═╝╚═════╝░░╚════╝░░╚════╝░╚═╝╚═╝░░╚══╝░░░╚═╝░░░
#	
#	░██████╗██╗███╗░░░███╗██╗░░░██╗██╗░░░░░░█████╗░████████╗██╗░█████╗░███╗░░██╗
#	██╔════╝██║████╗░████║██║░░░██║██║░░░░░██╔══██╗╚══██╔══╝██║██╔══██╗████╗░██║
#	╚█████╗░██║██╔████╔██║██║░░░██║██║░░░░░███████║░░░██║░░░██║██║░░██║██╔██╗██║
#	░╚═══██╗██║██║╚██╔╝██║██║░░░██║██║░░░░░██╔══██║░░░██║░░░██║██║░░██║██║╚████║
#	██████╔╝██║██║░╚═╝░██║╚██████╔╝███████╗██║░░██║░░░██║░░░██║╚█████╔╝██║░╚███║
#	╚═════╝░╚═╝╚═╝░░░░░╚═╝░╚═════╝░╚══════╝╚═╝░░╚═╝░░░╚═╝░░░╚═╝░╚════╝░╚═╝░░╚══╝
#	
#	░██████╗░█████╗░██████╗░██╗██████╗░████████╗
#	██╔════╝██╔══██╗██╔══██╗██║██╔══██╗╚══██╔══╝
#	╚█████╗░██║░░╚═╝██████╔╝██║██████╔╝░░░██║░░░
#	░╚═══██╗██║░░██╗██╔══██╗██║██╔═══╝░░░░██║░░░
#	██████╔╝╚█████╔╝██║░░██║██║██║░░░░░░░░██║░░░
#	╚═════╝░░╚════╝░╚═╝░░╚═╝╚═╝╚═╝░░░░░░░░╚═╝░░░

# Main bash script. Reference algorithm in flowchart.pdf file
# - State 0: first architecture configuration, file initializaztion and programs compiling
# - State 1: configuration choice using the disjoint algorithm
# - State 2: architecture reconfiguration with chosen parameters
# - State 3: script execution and reward computation
# - State 4: parameters update and saving

# ----------------------------------------------------------------------------------------
# State 0: First architecture configuration, file initializaztion and programs compiling -
# ----------------------------------------------------------------------------------------
echo "--------------------------------------"
echo -e "\e[91mState 0:\e[39m initialization "
echo "--------------------------------------"
echo -e "\e[94m1)\e[39m starting configuration -> ";  ./cmake_configure.klessydra-m.gcc.sh  3 3 2 					# Starting architecture configuration
echo -e "\e[94m2)\e[39m values.cpp reset ";			  cp clean.txt ../libs/klessydra_lib/cb_libs/src/values.cpp 	# Values.cpp file is initialized as empty
echo -e "\e[94m3)\e[39m vcompile ->"; 				  make vcompile													# make vcompile for compiling rtl architecture 
echo -e "\e[94m4)\e[39m compile all programs ->";	  make disjoint_algorithm && make update_disjoint_algorithm && make perfbench.fft_accl		

# Initializing log file:
echo -n "Log.txt ">log.txt																							# initializing log.txt file
echo "- alpha: 0.5 - iterations number: 50 - Feature dimension: 14" >> log.txt
echo -n "Start time: " >>log.txt
date +"%H:%M:%S" >> log.txt

# D PARAM, used for mem_elaboration script
Context_dim=4

# Required_time & energy. You can specify some values to force limits
required_time=0
required_energy=0

# Number of simulation iterations
iteration_number=2

# Main Simulation loop:
for ((i=0;i<$iteration_number;i++))
do
	echo -e "\e[91m--------------------------------------"
	echo "Iteration --> $i"
	echo -e "--------------------------------------\e[39m"
	
	#Scelta operazione tra FFT e MatMul
	if ((($i % 2) == 0 ))
	then
		op=1
	else
		op=2
	fi
	#op=1
	
  # Info.hpp: write context vector and initialize other variables
	echo "#define operation $op" > ../libs/klessydra_lib/cb_libs/inc/info.hpp
 	echo "#define chosen_action 0" >> ../libs/klessydra_lib/cb_libs/inc/info.hpp			
	echo "static constexpr float reward = 0;" >> ../libs/klessydra_lib/cb_libs/inc/info.hpp
 
	# ------------------------------------------------------------
	# State 1: configuration choice using the disjoint algorithm -
	# ------------------------------------------------------------
	echo "--------------------------------------------------------------------"
	echo -e "\e[91mState 1:\e[39m choosing best configuration with disjoint algorithm"
	echo "--------------------------------------------------------------------"
  
  # optimized solution. Decomment if u want to start testing all configurations
  #if (($i < 12))
  #then
  #  choice=$i
  #else
      
  	make disjoint_algorithm.vsimc > choice.txt										# Using algorithm parameters the best configuration is chosen and wrote on choice.txt
  	hexaddr=$(cat choice.txt| tail -n 4|head -n 1| cut -d ':' -f 2|cut -d 'x' -f 2)	# Read initial memory address from script
  	addr=$(printf "%d" $((16#$hexaddr)))											# Convert in decimal
  	./mem_elaboration.sh $Context_dim $addr > choice2.txt										# Format mem.txt files
  	./update1.sh 																	# Values.cpp file update
  
  	# M, F & D values related to the chosen configuration
  	choice=$(cat choice.txt| tail -n 2|head -n 1| cut -d ':' -f 2)		# Chosen configuration can be read from choice.txt file
  #fi
  
	case $choice in
		0)
      if (($op==1))
      then
			  M=1; F=1; D=1; freq=144.4; energy=31.712;
      else
        M=1; F=1; D=1; freq=144.4; energy=699.060;
      fi 
      ;;
		1)
       if (($op==1))
        then
			   M=1; F=1; D=2; freq=146;   energy=24.621;
       else
         M=1; F=1; D=2; freq=146;   energy=578.360;
       fi
       ;;
		2)
       if (($op==1))
        then
			   M=1; F=1; D=4; freq=137.2; energy=24.409;
       else
         M=1; F=1; D=4; freq=137.2; energy=581.185;
       fi
       ;;
			
		3)
       if (($op==1))
        then
			   M=1; F=1; D=8; freq=137.7; energy=30.177;
       else
         M=1; F=1; D=8; freq=137.7; energy=678.210;
       fi
       ;;
						
		4)
       if (($op==1))
        then
			   M=3; F=3; D=1; freq=148.2; energy=24.718;
       else
         M=3; F=3; D=1; freq=148.2; energy=609.927;
       fi
       ;;
					
		5)
       if (($op==1))
        then
			   M=3; F=3; D=2; freq=131.7; energy=23.221;
       else
         M=3; F=3; D=2; freq=131.7; energy=522.672;
       fi   
       ;;
							
		6)
       if (($op==1))
        then
			   	M=3; F=3; D=4; freq=120;   energy=26.548;
       else
         	M=3; F=3; D=4; freq=120;   energy=546.077;
       fi      
		   ;;
		7)
       if (($op==1))
        then
			   	M=3; F=3; D=8; freq=105.1; energy=38.371;
       else
         	M=3; F=3; D=8; freq=105.1; energy=771.699;
       fi         
       ;;
						
		8)
       if (($op==1))
        then
			   	M=3; F=1; D=1; freq=117.2; energy=29.691;
       else
         	M=3; F=1; D=1; freq=117.2; energy=723.402;
       fi            
		   ;;
		9)
       if (($op==1))
        then
			   	M=3; F=1; D=2; freq=128.9; energy=25.486;
       else
         	M=3; F=1; D=2; freq=128.9; energy=587.850;
       fi               
		  ;;
		10)
       if (($op==1))
        then
			   	M=3; F=1; D=4; freq=122;   energy=27.695;
       else
         	M=3; F=1; D=4; freq=122;   energy=591.815;
       fi               
      ;;
			
		11)
       if (($op==1))
        then
			   	M=3; F=1; D=8; freq=108.6; energy=41.545;
       else
         	M=3; F=1; D=8; freq=108.6; energy=774.500;
       fi               
      ;;
	esac																
	echo -e "\e[94mChosen configuration:\e[39m $choice"
	echo -e "- M: $M\n- F: $F\n- D: $D\n"

	# --------------------------------------------------------------
	# State 2: architecture reconfiguration with chosen parameters -
	# --------------------------------------------------------------
	echo "--------------------------------------------------------------------"
	echo -e "\e[91mState2:\e[39m architecture reconfiguration"
	echo "--------------------------------------------------------------------"
	./cmake_configure.klessydra-m.gcc.sh $M $F $D

	# --------------------------------------------------------------
	# State 3: script execution and reward computation			   -
	# --------------------------------------------------------------
	echo "--------------------------------------------------------------------"
	echo -e "\e[91mState 3:\e[39m script execution and reward"
	echo "--------------------------------------------------------------------"
	if (($op==1))
	then
		echo "1) Running FFT script"
		make perfbench.fft_accl.vsimc > results.txt													# FFT script is executed and the output is written on results.txt file
	else
		echo "1) Running MatMul script"
    	echo "#define mode $M">../libs/klessydra_lib/dsp_libs/inc/mode.h
		make matmul.vsimc > results.txt
	fi

	# Reward data: Clock cycle
	ck=$(cat results.txt |tail -n 3|head -n 1|cut -d ':' -f 2)									# Clock cycle info is read from the generated txt file (row -3)
 
  #Time computation
  time=$(echo "$ck $freq"|awk '{printf "%.6f \n", ($1/$2)}')
	echo "2) Reward computation"																      # Print of starting data
	echo "Requested clock cycle   ---> $ck"														# Necessary clock cycles
	echo "Frequency   ---> $freq"														          # Necessary clock cycles 
	echo "Energy ---> $energy"														            # Requested FPGA elements
  echo "Time --->   $time"														              # Total requested time
 
 
	# Reward: time normalization
	if(($required_time != 0))
	then
		max_ck=$required_time
   if(($time>$required_time))
   then
     norm_ck=0
   else
     norm_ck=$(echo "$time $max_ck $min_ck"|awk '{printf "%.6f \n", 1-($1-$3)/($2-$3)}') 		                # Clock cycles normalized with max min technique
   fi
	else
		if (($op==1))
		then
			min_ck=126.8960
      max_ck=228.4349 											# Starting max and min for clock cycle normalization (conv 4x4)
		else
			min_ck=2741.350
      max_ck=5793.3656													# Starting max and min for clock cycle normalization (conv 4x4)
		fi
   norm_ck=$(echo "$time $max_ck $min_ck"|awk '{printf "%.6f \n", 1-($1-$3)/($2-$3)}') 		                # Clock cycles normalized with max min technique
	fi
	

	# Reward: Energy normalization
	if(($required_energy!=0))
	then
		max_en=$required_energy
    if(($energy>$required_energy))
    then
      norm_en=0
    else
      norm_en=$(echo "$energy $max_en $min_en"|awk '{printf "%.6f \n", 1-(($1-$3)/($2-$3))}') 			          # Area data normalization with max min technique
    fi
	else
	  if (($op==1))
		then
			max_en=41.545
			min_en=23.221														                                        # Starting max and min for fpga elements normalization
		else
			max_en=774.5
			min_en=522.672															                                    # Starting max and min for fpga elements normalization (conv 4x4)
		fi
   norm_en=$(echo "$energy $max_en $min_en"|awk '{printf "%.6f \n", 1-(($1-$3)/($2-$3))}') 			          # Area data normalization with max min technique
	fi

	

  # Reward computation and normalization
  if (($op==1))
  then
    max_reward=0.993109
    min_reward=0.268309
  else
    max_reward=0.970782
    min_reward=0.149785
  fi
												                                      # Starting max and min for reward normalization
  reward=$(echo "$norm_ck $norm_en "|awk '{printf "%.6f \n", (0.5*$1 + 0.5*$2)}') 			              # Reward computation starting from clock cycle and fpga elements number
	norm_reward=$(echo "$reward $max_reward $min_reward"|awk '{printf "%.6f \n", (($1-$3)/($2-$3))}') # Reward max-min normalization to use the full range 0-1

	# Write infos on chosen action and reward on info.hpp
	echo -e "\e[94mComputed reward:\e[39m $norm_reward"											# Reward is displayed
  	echo "#define operation $op" > ../libs/klessydra_lib/cb_libs/inc/info.hpp
	echo "#define chosen_action $choice" >> ../libs/klessydra_lib/cb_libs/inc/info.hpp			# Chosen action and reward are written in header file info.hpp
	echo "static constexpr float reward = $norm_reward;" >> ../libs/klessydra_lib/cb_libs/inc/info.hpp
  
  # Set again the configuration used to run our script
  ./cmake_configure.klessydra-m.gcc.sh 3 3 2

	# --------------------------------------------------------------
	# State 4: model parameters update and save			   -
	# --------------------------------------------------------------
	echo "--------------------------------------------------------------------"
	echo -e "\e[91mState 4:\e[39m parameters update and saving"
	echo "--------------------------------------------------------------------"
	make update_disjoint_algorithm.vsimc > choice.txt

	# Values.cpp file update:
	hexaddr=$(cat choice.txt| tail -n 2|head -n 1| cut -d ':' -f 2|cut -d 'x' -f 2)	
	addr=$(printf "%d" $((16#$hexaddr)))											
	./mem_elaboration.sh $Context_dim $addr > choice2.txt										
	./update1.sh && echo -e "\e[94mUpdating:\e[39m the model parameter in values.cpp file have been updated"

	# Log.txt	
  if (($op==1))
	then
		echo "$i) FFT -> chosen_action: $choice -> time: $time, energy: $energy, norm_ck: $norm_ck, norm_en: $norm_en, reward: $reward, obtained reward: $norm_reward" >> log.txt
	else
		echo "$i) MatMul 64x64 -> chosen_action: $choice -> time: $time, energy: $energy, norm_ck: $norm_ck, norm_en: $norm_en, reward: $reward, obtained reward: $norm_reward" >> log.txt
	fi
   
done

echo -n "End time: " >>log.txt
date +"%H:%M:%S" >> log.txt
rm results.txt choice.txt choice2.txt
