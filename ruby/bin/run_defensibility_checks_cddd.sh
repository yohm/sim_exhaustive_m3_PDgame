#!/bin/bash
set -eux
ruby check_defensibility_pattern.rb 0_allD_defensible dddddd cddd > temp1
ruby check_defensibility_pattern.rb temp1 ddcddd cddd > temp2
ruby check_defensibility_pattern.rb temp2 dcdddd cddd > temp3
ruby check_defensibility_pattern.rb temp3 dccddd cddd > temp4
ruby check_defensibility_pattern.rb temp4 cddddd cddd > temp5
ruby check_defensibility_pattern.rb temp5 cdcddd cddd > temp6
ruby check_defensibility_pattern.rb temp6 ccdddd cddd > temp7
ruby check_defensibility_pattern.rb temp7 cccddd cddd > temp8
ruby count_strategies.rb temp1
ruby count_strategies.rb temp2
ruby count_strategies.rb temp3
ruby count_strategies.rb temp4
ruby count_strategies.rb temp5
ruby count_strategies.rb temp6
ruby count_strategies.rb temp7
ruby count_strategies.rb temp8
