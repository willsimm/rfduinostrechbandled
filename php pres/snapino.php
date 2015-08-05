
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>snapino table</title>

  <script src="https://code.jquery.com/jquery-1.10.2.js"></script>
	<style>
	table {
	    border-collapse: collapse;
	}

	table, td, th {
	    border: 1px solid black;
	}
	</style>




<script type="text/javascript">
function altRows(id){
	if(document.getElementsByTagName){  
		
		var table = document.getElementById(id);  
		var rows = table.getElementsByTagName("tr"); 
		 
		for(i = 0; i < rows.length; i++){          
			if(i % 2 == 0){
				rows[i].className = "evenrowcolor";
			}else{
				rows[i].className = "oddrowcolor";
			}      
		}
	}
}
window.onload=function(){
	altRows('alternatecolor');
}
</script>

<!-- CSS goes in the document HEAD or added to your external stylesheet -->
<style type="text/css">
table.altrowstable {
	font-family: verdana,arial,sans-serif;
	font-size:11px;
	color:#333333;
	border-width: 1px;
	border-color: #a9c6c9;
	border-collapse: collapse;
}
table.altrowstable th {
	border-width: 1px;
	padding: 8px;
	border-style: solid;
	border-color: #a9c6c9;
}
table.altrowstable td {
	border-width: 1px;
	padding: 8px;
	border-style: solid;
	border-color: #a9c6c9;
}
.oddrowcolor{
	background-color:#d4e3e5;
}
.evenrowcolor{
	background-color:#c3dde0;
}
</style>



</head>
<body>






	<table style="width:100%" class="altrowstable" id="alternatecolor">

<?php


//TAKE LAST 3 DIGITS OFF CSV TIMESTAMPS

$starttimestamp = 1438214400 ; //timestamp ;ast thursday
$timetrack = $starttimestamp ;


$days = array(1=> "Thursday",2=>"Friday",3=>"Saturday",4=>"Sunday",5 => "Monday", 6=> "Tuesday", 7=> "Wednesday",8=>"Thursday" );

echo $days[0];


for ($row=0;$row<=24;$row++){
	echo ("<tr>");
	for ($col=0;$col<=8;$col++){
		

		if ($col==0 && $row !=24){
			echo "<td id=''>";
			echo $row;
		} elseif  ($col==0 && $row ==24){
			echo "<td id=''>";
		}elseif ($row == 24 && $col !=0	){
			echo "<td id=''>";
			echo $days[$col];
		}else{

			echo "<td id='".$timetrack."'>";
			//echo $timetrack;
			$timetrack = $timetrack + 86400;

			
		}
		echo "</td>";

	}
	echo "</tr>";

	$timetrack = $starttimestamp + (($row+1) * 3600);
}
?>

</table>






<script>
$(document).ready(function() {
    $.ajax({
        type: "GET",
        url: "historic_data.txt",
        dataType: "text",
        success: function(data) {processData(data);}
     });
});

function processData(allText) {
    var allTextLines = allText.split(/\r\n|\n/);

    var uses = {};
    var totalLengthInteraction = {};

    for (var i=0; i<allTextLines.length; i++) {
        var data = allTextLines[i].split(',');
        var timeInteraction = parseInt(data[0]);
        var lengthInteraction = parseInt(data[1]);

        //console.log(timeInteraction);
        //console.log(lengthInteraction);

        // create a new javascript Date object based on the timestamp
		// multiplied by 1000 so that the argument is in milliseconds, not seconds
		var date = new Date(timeInteraction);
		var year = date.getFullYear();
		
		var month = date.getMonth();
		var day = date.getDate();
		var hours = date.getHours();
		//timestamp of the hour usage was recorded	
		var d = new Date(year, month, day, hours, 0, 0, 0);
		var dStamp = d.getTime();
		console.log(dStamp);
		//save number of usages that hour
		//save average usage length
		if (uses[dStamp]){

			uses[dStamp]++ ;
			totalLengthInteraction[dStamp]= totalLengthInteraction[dStamp] + lengthInteraction;

		}else{
			uses[dStamp] = 1;
			totalLengthInteraction[dStamp]=lengthInteraction;
		}




    }
    console.log(uses);
    console.log(totalLengthInteraction);

    jQuery.each( uses, function( key, value ) {
  		console.log( key/1000 + ": " + value );
    	var cellref = key/1000;
	    

	    var averageLengthInteraction =  totalLengthInteraction[key] / value;

	    var html = "".concat(value).concat( " av ").concat(averageLengthInteraction);

	    $("#".concat(cellref)).text(html);
	    $("#".concat(cellref)).css('background-color','white');






	});


}


</script>
 
</body>
</html>