<template>
    <v-img src="@/assets/history.jpg" alt=""
           gradient="to bottom, rgba(200,10,50,.1), rgba(250,100,10, 0.4)"
           width="100%"
           height="100%">

        <v-card max-width="700"
                class="mx-auto my-1"
                color="rgb(0, 0, 0, 0)">
            <strong class="white--text mx-3 red  darken-4">
                {{prisoneers.length}}  PLAYERS
            </strong>
            <v-virtual-scroll :items="prisoneers"
                              height="900"
                              item-height="75">
                <template v-slot:default="{ item }">
                    <v-hover v-slot:default="{hover}">
                        <v-card color="rgb(10, 10, 10, 0.75)"
                                width="700px"
                                max-height="90px"
                                :elevation="hover ? 10:2"
                                shaped>

                            <v-card-title>
                                {{item.data.name}}
                                <v-chip class="mx-2" v-if="item.data.host" color="green" text-color="white">
                                    Host
                                    <v-icon>
                                        mdi-fire
                                    </v-icon>
                                </v-chip>
                                <v-spacer></v-spacer>
                                <a class="mx-3">{{item.data.ip}}</a>
                                <v-expand-transition>
                                    <div v-if="hover"
                                         class="transition-fast-in-fast-out darken-4 "
                                         style="height: 100%;">
                                        <v-btn v-if="!item.kickBtnLoadingAnimation"
                                               outlined color="success"
                                               @click="unblockPrisoneer(item)">
                                            Free
                                        </v-btn>
                                    </div>
                                </v-expand-transition>
                                <v-btn v-if="item.kickBtnLoadingAnimation"
                                       outlined color="success"
                                       loading>
                                    Free
                                </v-btn>

                            </v-card-title>
                        </v-card>
                    </v-hover>
                </template>
            </v-virtual-scroll>
        </v-card>
    </v-img>
</template>

<script>
    import axios from 'axios'
    export default {
        name: 'Prison',
        components: {

        },

        data() {
            return {
                prisoneers: [],
                apiURL: 'http://localhost:8000/api/',
            }
        },
        mounted() {
            axios.get(this.apiURL +'prison/').then(response => {
                
                this.prisoneers = response.data.map(player => {
                    return {
                        data: player,
                        kickBtnLoadingAnimation: false,
                    }
                });
                console.log(this.prisoneers)
            });
        },
        methods: {
            unblockPrisoneer: async function (prisoneer) {
                // loading animation
                prisoneer.kickBtnLoadingAnimation = true
                await new Promise(resolve => setTimeout(resolve, 500))
                
                // get the node server to remove the player from 'session'
                await axios.delete(this.apiURL + 'prison/' +prisoneer.data.ip + '/');

                // cleanup
                prisoneer.kickBtnLoadingAnimation = false
                this.prisoneers.splice(this.prisoneers.indexOf(prisoneer), 1);
            },
           
        }
    }
</script>