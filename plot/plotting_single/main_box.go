package main

import (
	"github.com/skiesel/expsys/rdb"
)

const (
	dataRoot   = "data/"
	plottype   = ".eps"
	plotWidth  = 5.
	plotHeight = 5.
)

func main() {
	Q_new()
}

func Q_new() {
	filters := map[string]map[string]string{
		//"RRT":    map[string]string{"planner": "RRT"},		
		"BEAST":  map[string]string{"planner": "BEAST"},
		"P-PRM":  map[string]string{"planner": "PlakuRRT"},
		"Gust":  map[string]string{"planner": "gust"},
		//"Gust_3reg": map[string]string{"planner": "gustold1"},
		//"Gust_0reg":  map[string]string{"planner": "gustold"},
		"KPIECE": map[string]string{"planner": "KPIECE"},
	}
	domainTypes := map[string][]string{
		//"2D": []string{"KinematicCar"},
		"2D": []string{"DynamicCar","Hovercraft"},
		"3D": []string{"Quadrotor","Blimp"},
		//"3D": []string{"Quadrotor"},		
		//"3D": []string{"Quadrotor","Blimp"},
	}

	maps := map[string][]string{
		//"2D": []string{"forest.dae"},
		"2D": []string{"forest.dae","3-ladder.dae","intersection.dae","parking-lot.dae","single-wall.dae"},
		//"3D": []string{"forest.dae", "single-wall.dae","3-ladder.dae","parking-lot.dae"},
		"3D": []string{"forest.dae", "fifthelement.dae"},
	}

	for domainType, domains := range domainTypes {

		for _, domain := range domains {

			mapDateSet := map[string][]*rdb.Dataset{}
			for _, mmap := range maps[domainType] {
				for key := range filters {
					filters[key]["timeout"] = "40"
					filters[key]["domain"] = domain
					filters[key]["map"] = mmap
				}

				dss := rdb.GetDatasetsFromNonRDBFormat(dataRoot, filters, true, nonRDBReader)
				mapDateSet[mmap] = dss
			}
			makeCompactBoxPlot(domain, "CPU Time", "Solving Time", plottype, mapDateSet, plotWidth, plotHeight)
		}
	}
}

//func Anytime() {
//	filters := map[string]map[string]string {
//		"SST" : map[string]string { "planner" : "SST" },
//		// "SST*" : map[string]string { "planner" : "SSTStar" },
//		// "Restarting RRT with Pruning" : map[string]string { "planner" : "RestartingRRTWithPruning" },
//		// "A-BEAST-4 (SST)" : map[string]string { "planner" : "AnytimeBEAST_4",
//		// 								"sstpruning" : "SST",
//		// 								"costpruning" : "G",
//		// 								"sampler" : "BEAST",
//		// 							},
//		// "A-BEAST-4 (SST*)" : map[string]string { "planner" : "AnytimeBEAST_4",
//		// 								"sstpruning" : "SSTStar",
//		// 								"costpruning" : "G",
//		// 								"sampler" : "BEAST",
//		// 							},
//		"A-BEAST-5 (SST)" : map[string]string { "planner" : "AnytimeBEAST_5",
//										// "sstpruning" : "SST",
//										// "costpruning" : "G",
//										// "sampler" : "BEAST",
//									},
//		// "A-BEAST-5 (SST*)" : map[string]string { "planner" : "AnytimeBEAST_5",
//		// 								"sstpruning" : "SSTStar",
//		// 								"costpruning" : "G",
//		// 								"sampler" : "BEAST",
//		// 							},
//	}

//	domainTypes := map[string][]string {
//		"2D" : []string{/*"KinematicCar", */"DynamicCar"/*, "Hovercraft"*/},
//		// "3D" : []string{"Quadrotor", "Blimp"},
//	}

//	maps := map[string][]string {
//		"2D" : []string{/*"forest.dae", "single-wall.dae", "3-ladder.dae", */"parking-lot.dae"/*, "intersection.dae"*/},
//		// "3D" : []string{"forest.dae", "fifthelement.dae"},
//	}

//	meshes := map[string][]string {
//		"KinematicCar" : []string{"car2_planar_robot.dae", "car2_planar_robot_SCALED.dae"},
//		"DynamicCar" : []string{"car2_planar_robot.dae", "car2_planar_robot_SCALED.dae"},
//		"Hovercraft" : []string{"car2_planar_robot.dae", "car2_planar_robot_SCALED.dae"},
//		"Quadrotor" : []string{"quadrotor.dae"},
//		"Blimp" : []string{"blimp.dae"},
//	}

//	for domainType, domains := range domainTypes {
//		for _, domain := range domains {
//			for _, mmap := range maps[domainType] {
//				for _, mesh := range meshes[domain] {
//					for key := range filters {
//						filters[key]["timeout"] = "60"
//						filters[key]["domain"] = domain
//						filters[key]["map"] = mmap
//						filters[key]["agent"] = mesh
//					}

//					dss := rdb.GetDatasetsFromNonRDBFormat(dataRoot, filters, true, nonAnytimeRDBReader)

//					// for i, ds := range dss {
//					// 	dss[i] = ds.FilterDataset(func(val string) bool {
//					// 		return datautils.ParseIntOrFail(val) <= 10
//					// 	}, "seed")
//					// }

//					title := fmt.Sprintf("%s - %s - %s", domain, strings.Replace(mmap, ".dae", "", -1), strings.Replace(mesh, ".dae", "", -1))

//					fmt.Println(title)

//					solvedCounts := map[string]float64{}
//					include := true
//					for _, ds := range dss {
//						if ds.GetSize() == 0 {
//							include = false
//						}

//						dsValues := ds.GetColumnValuesWithKey("solution", "inst", "solution cost")

//						count := 0.
//						for _, dfValues := range dsValues {
//							if len(dfValues[0]) > 0 {
//								count++
//							}
//						}

//						fmt.Printf("\t%s: %d / %d\n", ds.GetName(), int(count), ds.GetSize())

//						solvedCounts[ds.GetName()] = count / float64(len(dsValues))
//					}
//					fmt.Println()

//					if !include {
//						fmt.Println("Skipping!!\n")
//						continue
//					}

//					makeBarPlot(solvedCounts, title, ".", "Percent Solved", plottype, plotWidth, plotHeight)
//					makeAnytimePlot(dss, title, ".", "solution", "solution time", "solution cost", "CPU Time", "Solution Quality", plottype, 0, 60, 15, plotWidth, plotHeight)
//				}
//			}
//		}
//	}
//}

//func Quadrotor() {
//	filters := map[string]map[string]string {
//		"RRT" : map[string]string { "planner" : "RRT" },
//		"KPIECE" : map[string]string { "planner" : "KPIECE" },
//	}

//	// for sr := 2; sr <= 6; sr += 2 {
//		// for ed := 5; ed <= 10; ed += 5 {
//			// for prm := 1000; prm <= 10000; prm*=10 {

//	sr := 6
//	ed := 5
//	prm := 1000

//				label := "BEAST"//fmt.Sprintf("New\nsr=%d\ned=%d\nprm=%d", sr, ed, prm)
//				filters[label] = map[string]string {
//					"planner" : "NewPlanner",
//					"stateradius" : strconv.Itoa(sr),
//					"numprmedges" : strconv.Itoa(ed),
//					"prmsize" : strconv.Itoa(prm),
//				}

//	sr = 6
//	ed = 10
//	prm = 10000

//				label = "P-PRM"//fmt.Sprintf("PPRM\nsr=%d\ned=%d\nprm=%d", sr, ed, prm)
//				filters["P-PRM"] = map[string]string {
//					"planner" : "PlakuRRT",
//					"stateradius" : strconv.Itoa(sr),
//					"numprmedges" : strconv.Itoa(ed),
//					"prmsize" : strconv.Itoa(prm),
//				}
//			// }
//		// }
//	// }

//	for key := range filters {
//		filters[key]["timeout"] = "60"
//		filters[key]["domain"] = "Quadrotor"
//	}

//	dss := rdb.GetDatasetsFromNonRDBFormat(dataRoot, filters, true, nonRDBReader)

//	makeBoxPlot("Quadrotor", "CPU Time", "Solving Time", plottype, plotWidth, plotHeight, dss, false, false)
//}

//func Blimp() {
//	filters := map[string]map[string]string {
//		"RRT" : map[string]string { "planner" : "RRT" },
//		"KPIECE" : map[string]string { "planner" : "KPIECE" },
//	}

//	// for sr := 2; sr <= 6; sr += 2 {
//		// for ed := 5; ed <= 10; ed += 5 {
//			// for prm := 1000; prm <= 10000; prm*=10 {

//	sr := 6
//	ed := 5
//	prm := 1000

//				label := "BEAST"//fmt.Sprintf("New\nsr=%d\ned=%d\nprm=%d", sr, ed, prm)
//				filters[label] = map[string]string {
//					"planner" : "NewPlanner",
//					"stateradius" : strconv.Itoa(sr),
//					"numprmedges" : strconv.Itoa(ed),
//					"prmsize" : strconv.Itoa(prm),
//				}

//	sr = 6
//	ed = 10
//	prm = 10000

//				label = "P-PRM"//fmt.Sprintf("PPRM\nsr=%d\ned=%d\nprm=%d", sr, ed, prm)
//				filters[label] = map[string]string {
//					"planner" : "PlakuRRT",
//					"stateradius" : strconv.Itoa(sr),
//					"numprmedges" : strconv.Itoa(ed),
//					"prmsize" : strconv.Itoa(prm),
//				}
//			// }
//		// }
//	// }

//	for key := range filters {
//		filters[key]["timeout"] = "60"
//		filters[key]["domain"] = "Blimp"
//	}

//	dss := rdb.GetDatasetsFromNonRDBFormat(dataRoot, filters, true, nonRDBReader)

//	makeBoxPlot("Blimp", "CPU Time", "Solving Time", plottype, plotWidth, plotHeight, dss, false, false)
//}

//func DynamicCar() {
//	filters := map[string]map[string]string {
//		"RRT" : map[string]string { "planner" : "RRT" },
//		"KPIECE" : map[string]string { "planner" : "KPIECE" },
//	}

//	// for sr := 2; sr <= 6; sr += 2 {
//		// for ed := 5; ed <= 10; ed += 5 {
//			// for prm := 100; prm <= 10000; prm*=10 {

//	sr := 4
//	ed := 5
//	prm := 100

//				label := "BEAST"//fmt.Sprintf("New\nsr=%d\ned=%d\nprm=%d", sr, ed, prm)
//				filters[label] = map[string]string {
//					"planner" : "NewPlanner",
//					"stateradius" : strconv.Itoa(sr),
//					"numprmedges" : strconv.Itoa(ed),
//					"prmsize" : strconv.Itoa(prm),
//				}

//	sr = 4
//	ed = 10
//	prm = 10000

//				label = "P-PRM"//fmt.Sprintf("P-PRM\nsr=%d\ned=%d\nprm=%d", sr, ed, prm)
//				filters[label] = map[string]string {
//					"planner" : "PlakuRRT",
//					"stateradius" : strconv.Itoa(sr),
//					"numprmedges" : strconv.Itoa(ed),
//					"prmsize" : strconv.Itoa(prm),
//				}
//			// }
//		// }
//	// }

//	for key := range filters {
//		filters[key]["timeout"] = "60"
//		filters[key]["domain"] = "DynamicCar"
//	}

//	dss := rdb.GetDatasetsFromNonRDBFormat(dataRoot, filters, true, nonRDBReader)

//	makeBoxPlot("Dynamic Car", "CPU Time", "Solving Time", plottype, plotWidth, plotHeight, dss, false, false)
//}

//func KinematicCar() {
//	filters := map[string]map[string]string {
//		"RRT" : map[string]string { "planner" : "RRT" },
//		"KPIECE" : map[string]string { "planner" : "KPIECE" },
//	}

//	// for sr := 2; sr <= 6; sr += 2 {
//		// for ed := 5; ed <= 10; ed += 5 {
//			// for prm := 100; prm <= 10000; prm*=10 {

//	sr := 4
//	ed := 5
//	prm := 100

//				label := "BEAST"//fmt.Sprintf("New\nsr=%d\ned=%d\nprm=%d", sr, ed, prm)
//				filters[label] = map[string]string {
//					"planner" : "NewPlanner",
//					"stateradius" : strconv.Itoa(sr),
//					"numprmedges" : strconv.Itoa(ed),
//					"prmsize" : strconv.Itoa(prm),
//				}

//	sr = 4
//	ed = 10
//	prm = 10000

//				label = "P-PRM"//fmt.Sprintf("P-PRM\nsr=%d\ned=%d\nprm=%d", sr, ed, prm)
//				filters[label] = map[string]string {
//					"planner" : "PlakuRRT",
//					"stateradius" : strconv.Itoa(sr),
//					"numprmedges" : strconv.Itoa(ed),
//					"prmsize" : strconv.Itoa(prm),
//				}
//			// }
//		// }
//	// }

//	for key := range filters {
//		filters[key]["timeout"] = "60"
//		filters[key]["domain"] = "KinematicCar"
//	}

//	dss := rdb.GetDatasetsFromNonRDBFormat(dataRoot, filters, true, nonRDBReader)

//	makeBoxPlot("Kinematic Car", "CPU Time", "Solving Time", plottype, plotWidth, plotHeight, dss, false, false)
//}

//func Hovercraft() {
//	filters := map[string]map[string]string {
//		"RRT" : map[string]string { "planner" : "RRT" },
//		"KPIECE" : map[string]string { "planner" : "KPIECE" },
//	}

//	// for sr := 2; sr <= 6; sr += 2 {
//		// for ed := 5; ed <= 10; ed += 5 {
//			// for prm := 100; prm <= 10000; prm*=10 {

//	sr := 6
//	ed := 5
//	prm := 1000

//				label := "BEAST"//fmt.Sprintf("New\nsr=%d\ned=%d\nprm=%d", sr, ed, prm)
//				filters[label] = map[string]string {
//					"planner" : "NewPlanner",
//					"stateradius" : strconv.Itoa(sr),
//					"numprmedges" : strconv.Itoa(ed),
//					"prmsize" : strconv.Itoa(prm),
//				}

//	sr = 6
//	ed = 10
//	prm = 10000

//				label = "P-PRM"//fmt.Sprintf("P-PRM\nsr=%d\ned=%d\nprm=%d", sr, ed, prm)
//				filters[label] = map[string]string {
//					"planner" : "PlakuRRT",
//					"stateradius" : strconv.Itoa(sr),
//					"numprmedges" : strconv.Itoa(ed),
//					"prmsize" : strconv.Itoa(prm),
//				}
//			// }
//		// }
//	// }

//	for key := range filters {
//		filters[key]["timeout"] = "60"
//		filters[key]["domain"] = "Hovercraft"
//	}

//	dss := rdb.GetDatasetsFromNonRDBFormat(dataRoot, filters, true, nonRDBReader)

//	makeBoxPlot("Hovercraft", "CPU Time", "Solving Time", plottype, plotWidth, plotHeight, dss, false, false)
//}
