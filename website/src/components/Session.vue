<template>
        <v-img src="@/assets/session.jpg" alt=""
               gradient="to bottom, rgba(200,10,50,.1), rgba(250,100,10, 0.4)"
               width="100%"
               height="100%">

                <v-card max-width="700"
                        class="mx-auto my-1"
                        color="rgb(0, 0, 0, 0)">
                    <strong class="white--text mx-3 green  darken-4">
                        {{players.length}}  PLAYERS
                    </strong>
                    <v-virtual-scroll :items="players"
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
                                                       outlined color="error"
                                                       @click="kickPlayer(item)">
                                                    Kick
                                                </v-btn>
                                            </div>
                                        </v-expand-transition>
                                        <v-btn v-if="item.kickBtnLoadingAnimation"
                                               outlined color="error"
                                               loading>
                                            Kick
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
        name: 'Session',
        components: {

        },

        data(){
            return{
                players: [],
                apiURL: 'http://localhost:8000/api/',
            }
        },
        created: function () {
            setInterval(() => {
                this.updatez();
            }, 1000);
            // refresh every second
        },
        
        
        methods: {
            kickPlayer: async function (player) {
                //start loading animation
                player.kickBtnLoadingAnimation = true

                    // finish loading animation
                await new Promise(resolve => setTimeout(resolve, 500))
                player.kickBtnLoadingAnimation = false

                // get the node server to remove the player from 'session'
                await axios.delete(this.apiURL + 'players/' + player.data.ip +'/');

                // and also get him to put the kicked player in mw2 prison
                await axios.post(this.apiURL + 'prison/', player);
                //cleanup
                this.players.splice(this.players.indexOf(player), 1);
            },
            updatez: function () {

                axios.get(this.apiURL + 'players/').then(response => {
                    // add individual animationbutton for each player
                    //console.log(response.data)
                    this.players = response.data.map(player => {
                        return {
                            data: player,
                            kickBtnLoadingAnimation: false,
                        }
                    });
                });
            },

        }
    }
</script>